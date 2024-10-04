# Apro il file csv, prendo tutte le righe e mi stampo un plotter.

import csv 
from datetime import datetime
from collections import defaultdict
import matplotlib.pyplot as plt 

# creo un dizionario dove le chiavi sono gli intervalli di tempo, mentre i valori sono il conteggio degli ordini
# effettuati in quel determinato intervallo di tempo. 
dizionario_righe = defaultdict(int) 

def stampa_plotter():
    #apro il file 
    with open("./tempDB.csv", newline = "", encoding="utf-8") as tempDB:
        reader = csv.reader(tempDB)
        next(reader, None)                           # salto l'intestazione(prima riga) 
        for row in reader:
            tempo_in_secondi = converti(row[0])
            intervallo = (tempo_in_secondi // 2) * 2
            dizionario_righe[intervallo] += 1
        for interval, rows in dizionario_righe.items(): 
            print(f"Intervallo {interval}-{interval+1} secondi:")
            print(rows) 
        


def converti(tempo):
    time_obj = datetime.strptime(tempo, "%H:%M:%S")
    return time_obj.hour * 3600 + time_obj.minute * 60 + time_obj.second

#stampa_plotter()

seconds_intervals = sorted(dizionario_righe.keys())
for c, v in dizionario_righe.items():
    print("c: ", c)
    print("v: ", v)
order_counts = [dizionario_righe[interval] for interval in seconds_intervals] 


plt.figure(figsize=(10, 6))
plt.plot(seconds_intervals, order_counts, marker='o', linestyle='-', color='b')
plt.xlabel('Secondi')
plt.ylabel('Numero di ordini')
plt.title('Numero di ordini ogni 2 secondi')
plt.grid(True)

# Mostra il grafico
#plt.show()

dizio_righe = defaultdict(list) 

# nel file csv ho due colonne: start e delay. Faccio un grafico dove l'asse delle x è il tempo, mentre l'asse 
# delle y è la media dei ritardi per ogni secondo. 
def stampa_delay():
    with open("./LOG_Delay.csv", newline = "", encoding="utf-8") as delay:
        reader = csv.reader(delay) 
        next(reader, None)
        intervalli, ritardi_medi = [], []
        for row in reader:
            r = row[0].split() 
            tempo_in_secondi = converti(r[1]) % 100 
            intervallo = (tempo_in_secondi // 2) * 2
            dizio_righe[intervallo].append(row)   
        for interval, rows in dizio_righe.items():  
            tot, media = 0, 0.0 
            for element in rows:
                media += float(element[1])  
                tot += 1
            media = media / tot  
            intervalli.append(interval)
            ritardi_medi.append(media) 
            print(f"Intervallo {interval}-{interval+1} secondi:") 
            print([rows[0][0], media])     
        
        plt.figure(figsize= (10,6))
        plt.plot(intervalli, ritardi_medi, marker = 'o', linestyle = '-', color = 'b')
        plt.xlabel("Secondi")
        plt.ylabel("Ritardo medio")
        plt.title("Ritardo medio per intervallo di due secondi")
        plt.grid(True)
        plt.show() 

stampa_delay() 

