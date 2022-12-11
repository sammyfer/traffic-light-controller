from ftplib import FTP
import time
import requests
import os
import sys
import json
# Neural network imports
import numpy as np
from PIL import Image, ImageDraw
from pathlib import Path
import skimage
import tensorflow as tf
from tensorflow.python.client import device_lib
from mrcnn.config import Config
import mrcnn.utils as utils
from mrcnn import visualize
from mrcnn import tcc
import mrcnn.model as modellib


class InferenceConfig(tcc.CocoSynthConfig):
    GPU_COUNT = 1
    IMAGES_PER_GPU = 1
    IMAGE_MIN_DIM = 512
    IMAGE_MAX_DIM = 512
    DETECTION_MIN_CONFIDENCE = 0.99

def initNeuralNetwork():
    pathRaiz="/scripts"
    os.chdir(pathRaiz)

    msg = tf.constant('Hello, TensorFlow!')
    sess = tf.compat.v1.Session()
    print(sess.run(msg))

    device_lib.list_local_devices()
    ROOT_DIR = os.getcwd()
    assert os.path.exists(ROOT_DIR), 'ROOT_DIR does not exist. Did you forget to read the instructions above? ;)'

    EnderecoMask=os.path.join(pathRaiz, "Mask_RCNN")
    sys.path.append(EnderecoMask)

    print("================= Configuração da rede =================")
    configurate = tcc.CocoSynthConfig()
    dt = tcc.CocoLikeDataset()

    print("================== Carregar o Dataset ====================")
    EndTreinamento=os.path.join(pathRaiz, "Treinamento")
    global dataset_train
    dataset_train = dt
    dataset_train.load_data(
        os.path.join(EndTreinamento, "coco_instances.json"),
        os.path.join(EndTreinamento, "images")
    )
    dataset_train.prepare()

    EndValidacao=os.path.join(pathRaiz, "Validacao")
    dataset_val = dt
    dataset_val.load_data(
        os.path.join(EndValidacao, "coco_instances.json"),
        os.path.join(EndValidacao, "images")
    )
    dataset_val.prepare()

    MODEL_DIR = os.path.join(pathRaiz, "logs")
    print("MODEL_DIR= "+MODEL_DIR)

    inference_config = InferenceConfig()
    print("=============== Recreate the model in inference mode ===============")
    model = modellib.MaskRCNN(mode="inference",
                            config=inference_config,
                            model_dir=MODEL_DIR)
    model_path = model.find_last()
    assert model_path != "", "Provide path to trained weights"
    print("Loading weights from ", model_path)
    model.load_weights(model_path, by_name=True)
    return model

def initFtpClient():
    HOST='ec2-18-230-151-174.sa-east-1.compute.amazonaws.com'
    USER='esp32user'
    PASSWORD='esp32user123'
    ftp=FTP(host=HOST, user=USER, passwd=PASSWORD)
    ftp.encoding="utf-8"
    return ftp

def downloadFile(ftpinfo, tl):
    filename=ftpinfo['file']
    with open(filename, "wb") as file:
        # Command for Downloading the file "RETR filename"
        ftpinfo['ftp'].retrbinary(f"RETR semaforo_{tl}/{filename}", file.write)
    print(f'Baixado a imagem do semaforo {tl}')
    server='ec2-18-230-151-174.sa-east-1.compute.amazonaws.com'
    user='ec2-user'
    key='/home/ec2-user/.ssh/default-key.pem'
    container='nginx'
    os.system(f'rsync -Pavq -e "ssh -i {key}" {filename} {user}@{server}:/home/{user}')
    os.system(f'ssh -o StrictHostKeyChecking=no -i {key} {user}@{server} sudo docker cp /home/{user}/{filename} {container}:/usr/share/nginx/html/')
    print("Imagem enviada para nginx")

def openTl(tl):
    print(f'Abrindo Semaforo {tl}')
    sendResp(tl, 1)

def closeTl(tl):
    print(f'Fechando Semaforo {tl}')
    sendResp(tl, 0)

def sendResp(tl, resp):
    # Update file /usr/share/nginx/html/resp_semaforo_X.txt
    server='ec2-18-230-151-174.sa-east-1.compute.amazonaws.com'
    user='ec2-user'
    key='/home/ec2-user/.ssh/default-key.pem'
    script='/scripts/update_resp.sh'
    container='nginx'
    os.system(f'ssh -o StrictHostKeyChecking=no -i {key} {user}@{server} sudo docker exec -t {container} sh {script} {tl} {resp}')

def updateCont(cont):
    # Update file /usr/share/nginx/html/cont.txt
    server='ec2-18-230-151-174.sa-east-1.compute.amazonaws.com'
    user='ec2-user'
    key='/home/ec2-user/.ssh/default-key.pem'
    script='/scripts/update_cont.sh'
    container='nginx'
    os.system(f'ssh -o StrictHostKeyChecking=no -i {key} {user}@{server} sudo docker exec -t {container} sh {script} {cont}')

def getStatusTl():
    url1='http://ec2-18-230-151-174.sa-east-1.compute.amazonaws.com/resp_semaforo_1.txt'
    url2='http://ec2-18-230-151-174.sa-east-1.compute.amazonaws.com/resp_semaforo_2.txt'
    urlcont='http://ec2-18-230-151-174.sa-east-1.compute.amazonaws.com/cont.txt'
    resp1=requests.get(url1).text[0]
    resp2=requests.get(url2).text[0]
    cont=requests.get(urlcont).text.replace('\n', '')
    statusTl={'t1': resp1, 't2': resp2, 'cont': cont}
    return statusTl

def getOpenedTl():
    openedTl=1 if getStatusTl().get('t1') == '1' else 2
    return openedTl

def getClosedTl():
    closedtl=1 if getStatusTl().get('t1') == '0' else 2
    return closedtl

def recognizeImage(model, image_path):
    img = skimage.io.imread(image_path)
    img_arr = np.array(img)
    results = model.detect([img_arr], verbose=1)
    r = results[0]
    return True if len(r.get("rois")) > 0 else False


if __name__ == '__main__':
    model=initNeuralNetwork()
    print("########## Model criado ##########")
    ftp=initFtpClient()
    print("########## FTP iniciado ##########")
    img_path='imagem.jpg'
    ftpinfo={'ftp': ftp, 'file': img_path}
    while(True):
        print(getStatusTl())
        if(int(getStatusTl().get("cont")) == 0):
            print("CONT É ZERO!!!!! INICIO DO LOOP")
            updateCont('20')
            openedTl=getOpenedTl()
            closedTl=getClosedTl()
            print(f'SEMAFORO ABERTO: {str(openedTl)} - SEMAFORO FECHADO: {str(closedTl)}')
            closeTl(openedTl)
            openTl(closedTl)
            print(getStatusTl())
        while int(getStatusTl().get("cont")) > 0:
            print("CONT É MAIOR QUE ZERO!!!")
            closedTl=getClosedTl()
            print(f'SEMAFORO FECHADO: {str(closedTl)}')
            print("BAIXANDO IMAGEM DO SEMAFORO"+str(closedTl))
            downloadFile(ftpinfo, closedTl)
            recognitionResult=False
            try:
                recognitionResult=recognizeImage(model, img_path)
                print("RESULTADO DA ANALISE: "+str(recognitionResult))
            except Exception as e:
                print(e)
            if recognitionResult:
                print(f"TEM AMBULANCIA NO SEMAFORO {str(closedTl)}!!!!!!!!!!!!!")
                openedTl=getOpenedTl()
                closedTl=getClosedTl()
                closeTl(openedTl)
                openTl(closedTl)
                print(f'SEMAFORO ABERTO: {str(openedTl)} - SEMAFORO FECHADO: {str(closedTl)}')
                updateCont('20')
            if(recognitionResult == False):
                print('Nenhuma ambulancia no trafego')
                time.sleep(1)
                if int(getStatusTl().get("cont")) > 0:
                    print("CONT MAIOR QUE ZERO!!! DIMINUINDO O CONT")
                    updateCont(str(int(getStatusTl().get("cont")) - 1))
                else:
                    print("CONT É ZERO!!!!! FINAL DO LOOP")
                    updateCont('0')
                print(getStatusTl())
