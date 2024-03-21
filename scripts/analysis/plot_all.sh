#!/bin/bash

rundir=$1

if [[ -z "$rundir" ]];
then
    exit 1
fi

python3 scripts/analysis/plot_differences_per_time.py $1 --outfile $1/differences_per_time.png

if [ ! -f "$rundir/burndown-tuple-size-1.csv" ];
then
    python3 scripts/analysis/gp_plot_burndown.py --tuple-size 1 csv $1 > $1/burndown-tuple-size-1.csv
fi

if [ ! -f "$rundir/burndown-tuple-size-3.csv" ];
then
    python3 scripts/analysis/gp_plot_burndown.py --tuple-size 3 csv $1 > $1/burndown-tuple-size-3.csv
fi

python3 scripts/analysis/gp_plot_burndown.py --tuple-size 1 plot $1/burndown-tuple-size-1.csv --outfile $1/burndown-tuple-size-1.png
python3 scripts/analysis/gp_plot_burndown.py --tuple-size 3 plot $1/burndown-tuple-size-3.csv --outfile $1/burndown-tuple-size-3.png