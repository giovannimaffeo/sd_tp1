from datetime import datetime
import os
def validate(file_path):
    f = open(file_path, "r")
    lines = f.readlines()
    ids = []
    write_amount = {}
    ultima_hora = None
    for line in lines:
        line = (line.split('[')[1]).split('] ')
        id = int(line[0])
        if(id not in ids):
            ids.append(id)
            write_amount[id] = 1    
        else:
            write_amount[id] += 1
        str_hora = line[1]
        str_hora = str_hora[:-1]
        hora = datetime.strptime(str_hora, "%d/%m/%Y - %H:%M:%S.%f")
        if(ultima_hora!=None and hora<ultima_hora):
            print(ultima_hora)
            print(hora)
            raise Exception("Invalid results file: invalid write time sequence")
        ultima_hora = hora
    
    n = len(ids)
    nr = len(lines)
    old_amount = None
    for amount in write_amount.values():
        if(old_amount!=None and old_amount!=amount):
            raise Exception("Invalid results file: invalid write amounts for each client")
        old_amount = amount
    
    r = old_amount
    if(n*r!=nr):
        raise Exception("Invalid results file: invalid r writes for n clients")
    

    print(f"Results file was successfully validated for {file_path}")
    

if __name__ == "__main__":
    for file in os.listdir():
        if 'Resultados' in file:
            validate(file)