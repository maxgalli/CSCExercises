Igprof should be used in steps:
1) Produce the performance profile
2) Produce the human readable performance report

The commands to be used are therefore:
1) igprof -d -pp -z -o igprof.pp.gz myApp [arg1 arg2 ...]
2) igprof-analyse -d -v -g igprof.pp.gz >& igreport_perf.res

