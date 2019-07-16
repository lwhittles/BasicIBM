variable=["t",\
"N",\
"Deaths",\
"nbirths","UK","NUK"]

label=["Time in years and fractions thereof",\
"Total population size",\
"Number of deaths since last report",\
"Total number of births.",\
       "Total number of UK born",\
       "Total number of non-UK born"]

pound = '\xa3'
myfile=open('plotres.gnu','w')
myfile.write('set term postscript enhanced color \"Helvetica Bold\" 16')
myfile.write('\n')
myfile.write('set output \'individualplots2.ps\'')
myfile.write('\n')
myfile.write('set xtic nomirror')
myfile.write('\n')
myfile.write('set ytic nomirror')
myfile.write('\n')
myfile.write('set colors classic')
myfile.write('\n')
myfile.write('set xlabel \'Year\' font \"Helvetica,20\"')
myfile.write('\n')
myfile.write('set xrange [1981:2050]')
myfile.write('\n')
myfile.write('set yrange [:]')
for i in range(1,len(variable)):
    myfile.write('\n')
    y=variable.index(variable[i])+1
    x=variable.index("t")+1
    myfile.write('set ylabel \''+str(label[y-1])+'\' font \"Helvetica,20\"')
    myfile.write('\n')
    myfile.write('plot "../output/summary_100.txt" u '+str(x)+':'+str(y)+' w l lw 6 lt 1 lc -1 ti \'0\',\
"../output/summary_101.txt" u '+str(x)+':'+str(y)+' w l lw 6 lt 1 lc 1 ti \'1\'')

    myfile.write('\n')
myfile.write('set term x11')
myfile.write('\n')
myfile.close()

#from plotgraph import run_gnuplot
from plotgraph import *
#run_gnuplot('plotres.gnu','individualplots2.ps')
cygwin('gnuplot plotres.gnu')
cygwin('ps2pdf individualplots2.ps individualplots2.pdf')
