#!/usr/bin/env python3
"""Tries to explain Oura's recovery_high (minutes of high recovery in the day) from heart rate and the
day's other numbers, in a ring.db holding a cloud import with heart rate. It cannot: see
docs/oura-score-extraction.md ("Recovery"). Needs scikit-learn.

    python3 recovery_probe.py ring.db
"""
import os, sys, json, sqlite3, datetime
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import numpy as np, stress_fit as sf
from sklearn.ensemble import GradientBoostingRegressor
from sklearn.linear_model import Ridge
DB = sys.argv[1]
days,hrv=sf.load(DB)
c=sqlite3.connect('file:%s?mode=ro'%DB,uri=True)
rec={};
for (j,) in c.execute("select json from cloud_docs where kind='daily_stress'"):
    d=json.loads(j); rec[d['day']]=(d.get('recovery_high') or 0)/300
act={};sl={};rd={}
for (j,) in c.execute("select json from cloud_docs where kind='daily_activity'"):
    d=json.loads(j); act[d['day']]=d
for (j,) in c.execute("select json from cloud_docs where kind='sleep'"):
    d=json.loads(j)
    if d.get('type')=='long_sleep': sl[d['day']]=d
for (j,) in c.execute("select json from cloud_docs where kind='daily_readiness'"):
    d=json.loads(j); rd[d['day']]=d
rows=[];Y=[];names=[]
def g(d,k,default=0):
    v=d.get(k) if d else None
    return default if v is None else v
for i,dd in enumerate(days):
    f=sf.features(days,hrv,i)
    if not f: continue
    day=dd['day']; a=act[day]; s=sl.get(day); r=rd.get(day)
    b=dd['bins']; med=np.median(b)
    wd=datetime.date.fromisoformat(day).weekday()
    extra=[len(b), np.std(b), (np.percentile(b,10)-med), (np.percentile(b,90)-med), wd>=5,
           g(a,'steps')/1000, g(a,'active_calories')/100, g(a,'sedentary_time')/3600, g(a,'average_met_minutes'), g(a,'inactivity_alerts'),
           g(s,'average_hrv'), g(s,'lowest_heart_rate'), g(s,'average_heart_rate'), g(s,'total_sleep_duration')/3600, g(s,'efficiency'),
           g(s,'deep_sleep_duration')/3600, g(s,'rem_sleep_duration')/3600, g(s,'restless_periods'),
           g(r,'temperature_deviation'), ]
    rows.append(f+extra); Y.append(rec[day]); names.append(day)
X=np.array(rows,float); Y=np.array(Y); n=len(Y); sp=int(n*.7)
print(n,'days; mean rec slots',Y.mean(),' test mean-only MAE',np.mean(np.abs(Y[sp:]-Y[:sp].mean())))
def rep(name,m,cols=None):
    cc=slice(None) if cols is None else cols
    m.fit(X[:sp][:,cc],Y[:sp]); p=m.predict(X[sp:][:,cc]); print(name,'MAE',round(np.mean(np.abs(p-Y[sp:])),2),'corr',round(np.corrcoef(p,Y[sp:])[0,1],3))
    return m
nf=len(rows[0])
rep('ridge HR feats only',Ridge(3.0),list(range(13)))
rep('ridge all',Ridge(3.0))
m=rep('GBM all',GradientBoostingRegressor(n_estimators=200,max_depth=3,learning_rate=0.05,subsample=0.8,random_state=1))
rep('GBM HR feats only',GradientBoostingRegressor(n_estimators=200,max_depth=3,learning_rate=0.05,subsample=0.8,random_state=1),list(range(13)))
labels=['1','n','up4','up8','up12','up16','up20','up26','dn4','dn8','dn12','med','hrvz','nbins','std','p10','p90','weekend','steps','kcal','sed','met','alerts','hrv','rhr','avghr','sleeph','eff','deep','rem','restless','tempdev']
imp=sorted(zip(m.feature_importances_,labels),reverse=True)[:10]
print([(l,round(v,3)) for v,l in imp])
