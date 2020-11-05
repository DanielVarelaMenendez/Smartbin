import serial
def enviar(comando,ser): #función enviar , recibe el caracter a enviar (comando) y el
    objeto serial (ser) que puede hacer referencia tanto al mega como al esp32
    ser.write(bytes(comando,'utf-8'))#escribe en el serial el comando
    print('enviado',comando)
    while (not ser.read().decode('utf-8')=='F'): #mientras no reciba una 'F' no sale de este bucle
        print('Ejecutando accion')
    print('F')