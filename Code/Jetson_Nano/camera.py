import cv2 #librería para controlar la camara
import time #libreria para gestionar el tiempo

def gstreamer_pipeline( #caracteristicas de captura y visualizacion para detectar el movimiento
    capture_width=320, #baja resolución para reducir la carga de calculo (recorrre pixel a pixel comprobando si ha cambiado)
    capture_height=240,
    display_width=320,
    display_height=240,
    framerate=15, #baja tasa de fotogramas por segundo, no es una factor
    determinante solamente para visualizar la camara
    flip_method=0,
):
    return (
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "width=(int)%d, height=(int)%d, "
        "format=(string)NV12, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
        % (
        capture_width,
        capture_height,
        framerate,
        flip_method,
        display_width,
        display_height,
    )
)

def gstreamer_pipeline_captura( #caracteristicas para la captura de la imagen del residuo
    capture_width=1920, #resolución alta pues será sobre esta imagen sobre la que se realice la clasificacion
    capture_height=1080,
    display_width=1920,
    display_height=1080,
    framerate=30, #no determinante
    flip_method=0,
    ):
    return (
        "nvarguscamerasrc ! "
        "video/x-raw(memory:NVMM), "
        "width=(int)%d, height=(int)%d, "
        "format=(string)NV12, framerate=(fraction)%d/1 ! "
        "nvvidconv flip-method=%d ! "
        "video/x-raw, width=(int)%d, height=(int)%d, format=(string)BGRx ! "
        "videoconvert ! "
        "video/x-raw, format=(string)BGR ! appsink"
    % (
        capture_width,
        capture_height,
        framerate,

        flip_method,
        display_width,
        display_height,
    )
    )
    cap = cv2.VideoCapture(gstreamer_pipeline(flip_method=0), cv2.CAP_GSTREAMER) # se ejecuta el metodo VideoCapture para iniciar la camara

def detectar_mov(): #metodo para detectar movimiento
    sensibilidad=30000 #cantidad de pixeles que deben de cambiar para que se considere que ha habido movimiento
    if cap.isOpened():# si esta activa la camara
        window_handle = cv2.namedWindow("Smartbin", cv2.WINDOW_AUTOSIZE)#genera una
        ventana para visualizar la camara
        ret1, img1 = cap.read() #lee una imagen y la guarda en el vector img1
        mov=False
        # Window
        T0= time.time() #tiempo acutal
        while cv2.getWindowProperty("Smartbin", 0) >= 0:
            diferencia = 0 #variable donde guardar la cantidad de pixeles diferentes
            ret2, img2 = cap.read() #guarda otro fotograma
            cv2.imshow("Smartbin", img2) #lo muestra
            frameDelta = cv2.absdiff(img1, img2) #calcula la diferencia entre dos
            vectores en este caso los pixeles que han cambiado entre la imagen inicial y la
            actual
            #y guarda el vector diferencia en frameDelta
            thresh = cv2.threshold(frameDelta, 25, 255, cv2.THRESH_BINARY)[1]
            #binariza la diferencia (frameDelta) para poder sumar los pixeles mas facilmente
            if(thresh.sum()>sensibilidad and (time.time()-T0)>2): #suma los pixelesque han cambiado
            # y si es mayor que la sensibilidad y pasaron mas de 2s (tiempo para que la camara se inicie correctamente) ha detectado un movimiento
                print("Movimiento detectado!")
                break
            else:
                print("No hay movimiento")
            keyCode = cv2.waitKey(30) & 0xFF #si se pulsase la tecla ESC
            if keyCode == 27:
                break #se sale del bucle
            img1=img2 #si no se ha detectado movimiento la imagen de referencia es la anterior
        cap.release() #se deja de usar la camara
        cv2.destroyAllWindows() #se destruyen todas las ventanas
    else:
         print("No se pudo abrir la camara") #si no es capaz de abrir la camara
    return mov


def capturar(ruta): #metodo para caputrar imagen
    cap = cv2.VideoCapture(gstreamer_pipeline_captura(flip_method=0),
    cv2.CAP_GSTREAMER) #inicia la camara con los parametros de captura
    ret, img = cap.read() #lee la imagen
    cv2.imwrite(ruta, img) #guarda la imagen en la ruta proporcionada