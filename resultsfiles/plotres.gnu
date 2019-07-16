set term postscript enhanced color "Helvetica Bold" 16
set output 'individualplots2.ps'
set xtic nomirror
set ytic nomirror
set colors classic
set xlabel 'Year' font "Helvetica,20"
set xrange [1981:2050]
set yrange [:]
set ylabel 'Total population size' font "Helvetica,20"
plot "../output/summary_100.txt" u 1:2 w l lw 6 lt 1 lc -1 ti '0',"../output/summary_101.txt" u 1:2 w l lw 6 lt 1 lc 1 ti '1'

set ylabel 'Number of deaths since last report' font "Helvetica,20"
plot "../output/summary_100.txt" u 1:3 w l lw 6 lt 1 lc -1 ti '0',"../output/summary_101.txt" u 1:3 w l lw 6 lt 1 lc 1 ti '1'

set ylabel 'Total number of births.' font "Helvetica,20"
plot "../output/summary_100.txt" u 1:4 w l lw 6 lt 1 lc -1 ti '0',"../output/summary_101.txt" u 1:4 w l lw 6 lt 1 lc 1 ti '1'

set ylabel 'Total number of UK born' font "Helvetica,20"
plot "../output/summary_100.txt" u 1:5 w l lw 6 lt 1 lc -1 ti '0',"../output/summary_101.txt" u 1:5 w l lw 6 lt 1 lc 1 ti '1'

set ylabel 'Total number of non-UK born' font "Helvetica,20"
plot "../output/summary_100.txt" u 1:6 w l lw 6 lt 1 lc -1 ti '0',"../output/summary_101.txt" u 1:6 w l lw 6 lt 1 lc 1 ti '1'
set term x11
