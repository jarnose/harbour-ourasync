#!/usr/bin/env python3
"""Fits Oura's activity score from the daily_activity documents of a cloud import (a ring.db).

    python3 activity_fit.py ring.db

Prints, per contributor, how well a simple rule reproduces Oura's value and what the score
comes to when the rules stand in for the app. The rules (found by comparing 878 real days):

  move_every_hour      a table of the day's inactivity alerts: 100, 95, 78, 60, 43, 25, 1
  meet_daily_targets   the number of the last 7 days (today included) whose active calories
                       reached the target: 1, 25, 43, 60, 78, 95, 100, 100
  training_frequency   the number of the last 7 days with 150 or more medium+high activity
                       MET-minutes: 1, 40, 71, 96, 100
  stay_active          a falling curve of the day's sedentary hours (about +-3 of noise)
  training_volume      a rising curve of the last week's medium+high MET-minutes (rough)
  recovery_time        mostly 100; what lowers it is not found
  score                25 meet_daily_targets + 10 move_every_hour + 25 recovery_time
                       + 15 stay_active + 10 training_frequency + 15 training_volume, over 100
"""
import bisect
import collections
import datetime
import json
import sqlite3
import statistics
import sys


def docs(kind):
    c = sqlite3.connect(sys.argv[1])
    out = {}
    for day, j in c.execute("select day,json from cloud_docs where kind=? order by day", (kind,)):
        out[day] = json.loads(j)
    return out


A=docs('daily_activity'); days=sorted(A)
def prev(d,n): return (datetime.date.fromisoformat(d)-datetime.timedelta(days=n)).isoformat()
F=lambda r:r['high_activity_met_minutes']+r['medium_activity_met_minutes']
MEET=[1,25,43,60,78,95,100,100]
FREQ=[1,40,71,96,100,100,100,100]
MOVE=[100,95,78,60,43,25,1,1,1,1]
def lookup_curve(pairs,bins):
    # median per x-bin, then piecewise-linear interpolation
    buckets=collections.defaultdict(list)
    for x,y in pairs: buckets[int(x//bins)].append(y)
    pts=sorted(((k+0.5)*bins,statistics.median(v)) for k,v in buckets.items() if len(v)>=3)
    xs=[p[0] for p in pts]; ys=[p[1] for p in pts]
    def f(x):
        if x<=xs[0]: return ys[0]
        if x>=xs[-1]: return ys[-1]
        i=bisect.bisect_left(xs,x); t=(x-xs[i-1])/(xs[i]-xs[i-1]); return ys[i-1]+t*(ys[i]-ys[i-1])
    return f
rows=[d for d in days if A[d].get('contributors') and all(v is not None for v in A[d]['contributors'].values()) and A[d].get('score') is not None]
stay=lookup_curve([(A[d]['sedentary_time']/3600,A[d]['contributors']['stay_active']) for d in rows],0.25)
def win(d,n): 
    ds=[prev(d,k) for k in range(n)]
    return [A[x] for x in ds] if all(x in A for x in ds) else None
vol_pairs=[]
for d in rows:
    w=win(d,7)
    if w: vol_pairs.append((sum(F(r) for r in w),A[d]['contributors']['training_volume']))
vol=lookup_curve(vol_pairs,60)
errs=collections.defaultdict(list); score_err=[]
for d in rows:
    w=win(d,7)
    if not w: continue
    r=A[d]; c=r['contributors']
    est={}
    est['move_every_hour']=MOVE[min(r['inactivity_alerts'],9)]
    est['meet_daily_targets']=MEET[sum(1 for x in w if x['active_calories']>=x['target_calories'])]
    est['training_frequency']=FREQ[sum(1 for x in w if F(x)>=150)]
    est['stay_active']=stay(r['sedentary_time']/3600)
    est['training_volume']=vol(sum(F(x) for x in w))
    est['recovery_time']=100
    for k,v in est.items(): errs[k].append(abs(v-c[k]))
    W={'meet_daily_targets':25,'move_every_hour':10,'recovery_time':25,'stay_active':15,'training_frequency':10,'training_volume':15}
    sc=sum(W[k]*est[k] for k in W)/100
    score_err.append(sc-r['score'])
for k,v in errs.items(): print(f'{k:20s} mean abs {statistics.mean(v):5.2f}  exact {sum(1 for x in v if x<0.5)/len(v):.0%}')
print('score: mean err %.2f mean abs %.2f within2 %.0f%% max %.1f'%(statistics.mean(score_err),statistics.mean(abs(e) for e in score_err),100*sum(1 for e in score_err if abs(e)<=2)/len(score_err),max(abs(e) for e in score_err)))
