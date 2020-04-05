rem Use this batchfile for build the Dos version with Borland C++ (5.01)
bcc -ms  -lA=16 -l3 -Ox -RT- -P-  ASBENCH.C ASPISCSI.C
