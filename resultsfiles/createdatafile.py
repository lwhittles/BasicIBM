import csv

def create_txt_file(input_file,description_file,output_file):
    filex = open(output_file, "w")
    if description_file==0:
        Dataset="Dataset:     Population sizes by age, sex and region of birth for 1981.\n"
        Description="Description: These are numbers taken from the 1981 census used to initialize\n"\
                      +"\tthe England and Wales model population for 1981.\n"
        Labels="Label r:\t      Region of birth, 0=Non-UK, 1=UK.\n"\
                      +"Label s:\t      Sex, 0=male, 1=female.\n"\
                      +"Label a:\t      Age class.\n"\
                      +"\t              0=<15\n"\
                      +"\t              1=15-24\n"\
                      +"\t              2=25-34\n"\
                      +"\t              3=35-44\n"\
                      +"\t              4=45-59\n"\
                      +"\t              5=60+\n\n"
        # Reference="Reference:\t\n"        
        filex.write(Dataset)
        filex.write(Description)
        filex.write(Labels)
       # file.write(Reference)
    else:
        #filed =open(description_file,"r")
        with open(description_file) as description_file:
            lines = description_file.readlines()
        for line in lines:
            filex.write(line)
            
    with open(input_file) as input_file:
        lines = ["|"+line.replace(",", "|") for line in input_file.readlines()]
        for line in lines:
            filex.write(line) 

    filex.close()
    

def create_csv_file(input_file,description_file,output_file):
    resultFile = open(output_file,'wb+')
    pen= csv.writer(resultFile, delimiter=',')

    filex =open(description_file,"wb+")
    
    with open(input_file) as input_file:
        lines = [line.replace("\n", "").replace("\'","") for line in input_file.readlines()]
        for line in lines:
            
            if "|" in line:            
                row = ''.join([line.replace("|", ",") for line in line]).split(',')
                pen.writerow(row)
            else:
                filex.write(line)
                filex.write("\n")
    resultFile.close()
    filex.close
                
            
#input_file="../births.txt"
#output_file="births_b.csv"
description_file="description.txt"
#create_csv_file(input_file,description_file,output_file)

output_file="../births_new.txt"
input_file="births_b.csv"
create_txt_file(input_file,description_file,output_file)
