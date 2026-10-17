#!/usr/bin/env python3
"""Fits Oura's readiness contributors from a cloud import (a ring.db) and reports how well they
and the score are reproduced. The curves printed by readiness_estimate.cpp come from here.

    python3 readiness_fit.py ring.db

Each contributor is a curve (bin medians) of the quantity that fitted best: previous_night of the
sleep score, body_temperature of the size of the temperature deviation, resting_heart_rate of the
night's lowest heart rate minus its 60-day median, hrv_balance of the last 7 nights' HRV over the
60 before, sleep_balance of the last 7 nights' sleep, recovery_index of the minutes from the lowest
heart rate to waking, activity_balance of the last week's active calories over the 60 days before,
sleep_regularity of the spread of the last 14 nights' midpoints. The score is a weighted sum with a
penalty for a low body temperature or resting heart rate.
"""
import bisect
import collections
import datetime
import json
import sqlite3
import statistics
import sys

DB = sys.argv[1]


def docs(kind):
    c = sqlite3.connect(DB)
    return {day: json.loads(j) for day, j in c.execute("select day,json from cloud_docs where kind=? order by day", (kind,))}


R=docs('daily_readiness'); S=docs('daily_sleep'); A=docs('daily_activity')
sl={}
c=sqlite3.connect(DB)
for day,j in c.execute("select day,json from cloud_docs where kind='sleep' order by day"):
    o=json.loads(j)
    if o.get('type')=='long_sleep': sl[day]=o
days=sorted(R)
def prev(d,n): return (datetime.date.fromisoformat(d)-datetime.timedelta(days=n)).isoformat()
def inv(pts):
    pts=sorted(pts); return sum(1 for i in range(1,len(pts)) if pts[i][1]<pts[i-1][1])
def hist(d,field,n,excl_today=True,fn=lambda o,f:o.get(f)):
    vals=[]
    for k in range(1 if excl_today else 0,n+1 if excl_today else n):
        o=sl.get(prev(d,k))
        if o and fn(o,field) is not None: vals.append(fn(o,field))
    return vals

def mae_fit(pts,bins=20):
    pts=sorted(pts)
    if len(pts)<30: return None
    n=len(pts); k=max(1,n//bins); err=0
    for i in range(0,n,k):
        chunk=pts[i:i+k]; m=statistics.median(y for _,y in chunk)
        err+=sum(abs(y-m) for _,y in chunk)
    return err/n
def evaluate(name,target,featfn,sign=1):
    pts=[]
    for d in days:
        ct=R[d]['contributors'].get(target)
        if ct is None: continue
        try: f=featfn(d)
        except Exception: f=None
        if f is None: continue
        pts.append((sign*f,ct))
    return (name,target,len(pts),inv(pts),round(mae_fit(pts) or -1,2))
def mean_hist(d,field,n,src=None,start=1,fn=None):
    src=src or sl
    vals=[]
    for k in range(start,start+n):
        o=src.get(prev(d,k))
        if o is None: continue
        v=fn(o) if fn else o.get(field)
        if v is not None: vals.append(v)
    return vals

def tod(iso):
    h,m=int(iso[11:13]),int(iso[14:16]); return (h*60+m) if h*60+m<720 else (h*60+m-1440)
def curve_from(pairs,nbins=24):
    pairs=sorted(pairs); n=len(pairs); k=max(3,n//nbins); pts=[]
    for i in range(0,n,k):
        ch=pairs[i:i+k]
        if len(ch)<3: continue
        pts.append((statistics.median(x for x,_ in ch),statistics.median(y for _,y in ch)))
    xs=[p[0] for p in pts]; ys=[p[1] for p in pts]
    def f(x):
        if x<=xs[0]: return ys[0]
        if x>=xs[-1]: return ys[-1]
        i=bisect.bisect_left(xs,x); t=(x-xs[i-1])/(xs[i]-xs[i-1]) if xs[i]>xs[i-1] else 0
        return ys[i-1]+t*(ys[i]-ys[i-1])
    return f,pts
def features(d):
    o=sl.get(d)
    if not o: return None
    F={}
    F['previous_night']=S[d]['score'] if d in S else None
    F['body_temperature']=-abs(R[d]['temperature_deviation']) if R[d].get('temperature_deviation') is not None else None
    v=sorted(mean_hist(d,'lowest_heart_rate',60,start=1))
    F['resting_heart_rate']=-(o['lowest_heart_rate']-v[len(v)//2]) if len(v)>=5 and o.get('lowest_heart_rate') else None
    a=mean_hist(d,'average_hrv',7,start=0); b=mean_hist(d,'average_hrv',60,start=7)
    F['hrv_balance']=statistics.mean(a)/statistics.mean(b) if len(a)>=2 and len(b)>=5 else None
    v=mean_hist(d,'total_sleep_duration',7,start=0); F['sleep_balance']=statistics.mean(v)/3600 if len(v)>=3 else None
    it=o['heart_rate']['items']; vals=[(x,i) for i,x in enumerate(it) if x]
    F['recovery_index']=(len(it)-min(vals)[1])*5 if vals else None
    pd=prev(d,1); F['previous_day_activity']=A[pd]['score'] if pd in A and A[pd].get('score') is not None else None
    a=[A[prev(d,k)]['active_calories'] for k in range(1,8) if prev(d,k) in A]
    b=[A[prev(d,k)]['active_calories'] for k in range(8,68) if prev(d,k) in A]
    F['activity_balance']=statistics.mean(a)/statistics.mean(b) if len(a)>=3 and len(b)>=7 and statistics.mean(b) else None
    vals=[]
    for k in range(0,14):
        oo=sl.get(prev(d,k))
        if oo: vals.append((tod(oo['bedtime_start'])+tod(oo['bedtime_end'])+(1440 if tod(oo['bedtime_end'])<tod(oo['bedtime_start']) else 0))/2)
    F['regularity']=-statistics.pstdev(vals) if len(vals)>=3 else None
    return F
KEYS=['activity_balance','body_temperature','hrv_balance','previous_day_activity','previous_night','recovery_index','resting_heart_rate','sleep_balance','sleep_regularity']
FEATKEY={'sleep_regularity':'regularity'}
feat={d:features(d) for d in days}
curves={}
for k in KEYS:
    fk=FEATKEY.get(k,k)
    pairs=[(feat[d][fk],R[d]['contributors'][k]) for d in days if feat[d] and feat[d][fk] is not None and R[d]['contributors'].get(k) is not None]
    curves[k]=curve_from(pairs)[0]
W=dict(zip(KEYS,[0.092,0.102,0.142,0.094,0.148,0.096,0.146,0.138,0.032]))
errs=[];cerr=collections.defaultdict(list)
for d in days:
    F=feat[d]
    if not F or R[d].get('score') is None: continue
    est={}
    for k in KEYS:
        fk=FEATKEY.get(k,k)
        v=F[fk]
        est[k]=curves[k](v) if v is not None else None
    if any(est[k] is None for k in KEYS): continue
    for k in KEYS:
        t=R[d]['contributors'].get(k)
        if t is not None: cerr[k].append(abs(est[k]-t))
    sc=sum(W[k]*est[k] for k in KEYS)-0.292*max(0,50-est['body_temperature'])-0.101*max(0,50-est['resting_heart_rate'])
    errs.append(sc-R[d]['score'])
for k in KEYS: print(f'{k:22s} mean abs {statistics.mean(cerr[k]):5.2f}')
print(len(errs),'days: score mean err %.2f mean abs %.2f within2 %.0f%% within5 %.0f%% max %.1f'%(statistics.mean(errs),statistics.mean(abs(e) for e in errs),100*sum(1 for e in errs if abs(e)<=2)/len(errs),100*sum(1 for e in errs if abs(e)<=5)/len(errs),max(abs(e) for e in errs)))
