from __future__ import absolute_import
from __future__ import division
from __future__ import print_function
import argparse
import numpy as np
from PIL import Image

def load_labels(filename): # función para cargar las etiquetas o clases en formato de texto
    with open(filename, 'r') as f:
        return [line.strip() for line in f.readlines()]

def clasificar(interpreter,imagen): #función a la que se le pasa la red (interpreter) y clasifica la imagen contenida en la ruta (imagen)
    input_details = interpreter.get_input_details() #guarda las caracteristicas de la entrada de la red
    output_details = interpreter.get_output_details() #guarda las caracteristicas de salida de la red
    # NxHxWxC, H:1, W:2
    height = input_details[0]['shape'][1] #altura que debe de tener la imagen para
    introducirla en la red (299)
    width = input_details[0]['shape'][2] #ancho que debe de tener la imagen para introducirla en la red (299)
    floating_model = input_details[0]['dtype'] == np.float32 #comprueba el que el tensor de entrada sea de tipo float 32
    label_file='/media/tfgdvm/pendrive/Smartbin-IA/output_labels.txt' #ruta donde se encuentran las etiquetas de cada clase (permite transformar la salida de la red en una clase de forma textual)
    input_mean=127.5
    input_std=127.5
    img = Image.open(imagen).resize((width, height)) # se abre la imagen alojada en la ruta proporcionada y se transforma su tamaño para adaptarlo al que acepta la red
    input_data = np.expand_dims(img, axis=0) #se añaden tantas dimensiones como tiene la imagen
    if floating_model:
        input_data = (np.float32(input_data) - input_mean) / input_std
    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke() #invoca al interprete del formato tf.lite
    output_data = interpreter.get_tensor(output_details[0]['index']) #extrae en forma de vector los valores que proporciona el tensor de la capa de salida
    results = np.squeeze(output_data)#elimina del vector output_data todos los subset de 1 dimension y
    #por tanto quedaría un vector de 5 posiciones con los porcentajes (del 0 al 1) de que pertenezca a cada una de las 5 clases de residuos
    top_k = results.argsort()[-5:][::-1] #en este caso son 5 clases
    labels = load_labels(label_file) #se cargan las etiquetas
    for i in top_k: #para cada una de las clases se visualiza el porcentaje de pertenecer a cada una de ellas
        if floating_model:
            print('{:08.6f}: {}'.format(float(results[i]), labels[i]))
        else:
            print('{:08.6f}: {}'.format(float(results[i] / 255.0), labels[i]))
    if max(results)<0.6: #si el porcentaje más alto de pertenecer a una clase no supera el 60% se considera que el residuo es orgánico
        material= 'Organico'
        print('No se reconoce este residuo')
    else: #de lo contrario
        material = labels[np.argmax(results,axis=-1)] #el residuo es de la clase que la red ha determinado que es
    print("Es ",material)
    return material #devuelve la clase a la que pertenece le residuo que se está procesando