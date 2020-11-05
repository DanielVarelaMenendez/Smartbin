import cv2
import threading
import time
def gstreamer_pipeline(
    capture_width=1280,
    capture_height=720,
    display_width=1280,
    display_height=720,
    framerate=30,
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

cap = cv2.VideoCapture(gstreamer_pipeline(flip_method=0), cv2.CAP_GSTREAMER)

def mostrar():
# To flip the image, modify the flip_method parameter (0 and 2 are the most
common)
    print(gstreamer_pipeline(flip_method=0))
    time.sleep(2)
    if cap.isOpened():
        window_handle = cv2.namedWindow("Smartbin", cv2.WINDOW_AUTOSIZE)
        # Window
        while cv2.getWindowProperty("Smartbin", 0) >= 0:
            e.wait()
            e.clear()
            ret2, img2 = cap.read()
            cv2.imshow("Smartbin", img2)
            e.set ()
            # This also acts as
            keyCode = cv2.waitKey(30) & 0xFF
            # Stop the program on the ESC key
            if keyCode == 27:
            break
        cap.release()
        cv2.destroyAllWindows()
        exit()
    else:
        print("Unable to open camera")

def capturar(ruta):
    ret, img = cap.read()
    cv2.imwrite(ruta, img)
if __name__ == "__main__":
    numplastico=38
    numvidrio=6
    nummetal=0
    numpapel=8
    numcarton=26
    #threading.Thread(target=mostrar,name='Hilo-mostrar_camara')
    entrada ='S'
    e=threading.Event()
    hilo = threading.Thread(target=mostrar,name='Hilo-mostrar_camara')
    hilo.start()
    e.set()
    while (not (entrada=='F')):
        entrada=input("Introduce el tipo de residuo Plastico->P || Papel->L || Vidrio->V || Metal->M || Carton->C || Finalizar->F ")
        if entrada=='P':
            ruta= '/media/tfgdvm/pendrive/Smartbin-IA/dataset propio/plastico/plastico'+str(numplastico) + '.jpg'
            e.wait()
            e.clear()
            print('capturando')
            time.sleep(1)
            capturar(ruta)
            e.set()
            numplastico+=1
        if entrada=='L':
            ruta= '/media/tfgdvm/pendrive/Smartbin-IA/dataset
            propio/papel/papel'+str(numpapel)+'.jpg'
            e.wait()
            e.clear()
            print('capturando')
            time.sleep(1)
            capturar(ruta)
            e.set()
            numpapel+=1
        if entrada=='C':
            ruta= '/media/tfgdvm/pendrive/Smartbin-IA/dataset
            propio/carton/carton'+str(numcarton)+'.jpg'
            e.wait()
            e.clear()
            print('capturando')
            time.sleep(1)
            capturar(ruta)
            e.set()
            numcarton+=1
        if entrada=='M':
            ruta= '/media/tfgdvm/pendrive/Smartbin-IA/dataset
            propio/metal/metal'+str(nummetal) + '.jpg'
            e.wait()
            e.clear()

            print('capturando')
            time.sleep(1)
            capturar(ruta)
            e.set()
            nummetal+=1
        if entrada=='V':
            ruta= '/media/tfgdvm/pendrive/Smartbin-IA/dataset propio/vidrio/vidrio'
            +str(numvidrio) + '.jpg'
            e.wait()
            e.clear()
            print('capturando')
            time.sleep(1)
            capturar(ruta)
            e.set()
            numvidrio+=1
        if entrada=='F':
            cap.release()
            cv2.destroyAllWindows()
            hilo.join()
            exit()