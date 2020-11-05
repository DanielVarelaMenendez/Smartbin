import camera #script para gestion de la camara
import classifier #script para realizar la inferencia sobre la red neuronal
import time #libreria para manejar tiempo
import tensorflow as tf #libreria de IA
import communications as com #script para gestion de comunicaciones
import serial #libreria para comunicaciones seria
import shutil #libreria para gestion de archivos en linux
num_procesados=[0,0,0,0,0,0]
modelo_ruta='/media/tfgdvm/pendrive/Smartbin-IA/modelos/modelo_lite/modelolite_1'
#ruta donde se encuentra el modelo (la imagen de la red neuronal) a importar
imagen_ruta ='/media/tfgdvm/pendrive/Smartbin-IA/residuos procesados/' #ruta dondeguardar las imagenes de los residuos procesados
interpreter = tf.lite.Interpreter(modelo_ruta) #se carga el modelo de la red
interpreter.allocate_tensors()
#si se establecen las conexiones serie tanto con el Arduino como con el ESP32 comienza el programa
with serial.Serial('/dev/serial/by-id/usb-1a86_USB2.0-Serial-if00-port0', 9600,
timeout=1) as mega:
with serial.Serial('/dev/serial/by-id/usb-Silicon_Labs_CP2102_USB_to_UART_Bridge_Controller_0001-if00-port0', 9600,timeout=1) as esp32:
    com.enviar('H',mega) # hace el homing en la papelera
    while True: #bucle que se ejecutará constantemente mientras la papelera permanezca encendida
        camara.detectar_mov() #detecta movimiento
        print ("Movimiento detectado")
        time.sleep(2) # espera 2 segundos a que el residuo se asienta para evitar
        posibles rebotes
        ruta = imagen_ruta + 'residuo_actual.jpg'
        print('capturando')
        camara.capturar (ruta) #guarda la imagen en el directorio ruta provisionalmente
        t = time.process_time() #guarda el tiempo actual
        material=clasificador.clasificar(interpreter,ruta) #clasifica la imagen en ruta mediante la red neuronal
        print("Tiempo de clasificacion:" , time.process_time() - t) #indica el tiempo de procesamiento de la red
        vector_aux=['P','C','M','V','O','L'] #vector auxiliar de iniciales de cada residuo (la 'L' corresponde al papel)
        material_int= vector_aux.index(material[0]) # transforma el material de un string a un int del 0 al 5
        #(Plastico(P)-> 0 ,Carton (C)->1 ,Metal (M)->2, Vidrio (V)-> 3, Organico (O)->4, Papel (L)-> 5)
        ruta2= imagen_ruta + material+ '/' + material + str(num_procesados[material_int])+ '.jpg' # se establece la ruta del residuo en cuestión
        shutil.move (ruta,ruta2) #se mueve la imagen del residuo procesado a sudirectorio corrrespondiente para generar asi una base de imagenes de residuos procesados
        num_procesados[material_int]+=1 #se incrementa el numero de residuos procesados de esa clase
        print("Procesando" , material)
        contenedor=material #el nombre del contenedor es igual al material salvo
        if material=='Papel':#salvo en el caso del papel que va al contenedor del carton
            contenedor='Carton'
        print("Girando selector a posicion del contenedor de " ,contenedor)
        com.enviar(contenedor[0],mega) #s envia al arduino mega la inicial del contenedor
        al que debe de enviar el residuo
        print("Abriendo compuertas")
        com.enviar('B',mega) #abre compuertas
        time.sleep(5) #espera 5 segundos a que caiga el residuo
        print("Cerrando compuertas")
        comandou = 'U' + str(vector_aux.index(contenedor[0])) # envía al ESP32 una 'U'
        seguido de un numero del 0 al 4 indicando el tipo de residuo
        print(comandou)
        com.enviar(comandou, esp32) #envia el comdando anterior para ejecutar la medicion
        de ultrasonidos y el posterior envio a traves de LoRa
        com.enviar('S',mega) #cierra compuertas

