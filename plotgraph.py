import os
import subprocess
def run_gnuplot(input_gnu_file,output_ps_file):

    #call gnuplot and run code
    if not os.system('gnuplot ' +input_gnu_file):
        #tell python where to find ps2pdf on cygwin
        #subprocess.check_output("C:/cygwin/bin/bash.exe ps2pdfwr "+output_ps_file)
        if not os.system('ps2pdf '+output_ps_file):
            print 'warning no graph plotted!'


            
def cygwin(command):
    """
    Run a Bash command with Cygwin and return output.
    """
    # Find Cygwin binary directory
    for cygwin_bin in [r'C:\cygwin\bin', r'C:\cygwin64\bin']:
        if os.path.isdir(cygwin_bin):
            break
    else:
        raise RuntimeError('Cygwin not found!')
    # Make sure Cygwin binary directory in path
    if cygwin_bin not in os.environ['PATH']:
        os.environ['PATH'] += ';' + cygwin_bin
    # Launch Bash
    p = subprocess.Popen(
        args=['bash', '-c', command],
        stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    p.wait()
    # Raise exception if return code indicates error
    if p.returncode != 0:
        raise RuntimeError(p.stderr.read().rstrip())
    # Remove trailing newline from output
    return (p.stdout.read() + p.stderr.read()).rstrip()
#cygwin('ps2pdf economicanalysisplots-ICER_100yrs.ps')

def run_gnuplot_laptop(input_gnu_file,output_ps_file):

    #call gnuplot and run code
    os.system('gnuplot ' +input_gnu_file)#cygwin('gnuplot ' +input_gnu_file)
    #tell python where to find ps2pdf on cygwin
    cygwin('ps2pdf '+output_ps_file)
            
