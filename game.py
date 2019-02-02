from subprocess import Popen, PIPE
import numpy as np
from PIL import Image
import time
import cv2

messi = Popen("./main", stdin=PIPE, stdout=PIPE)

def cout(command):
    messi.stdin.write(np.array(command, np.int32).tobytes())
    messi.stdin.flush()

def cin():
    return np.frombuffer(messi.stdout.read(4), np.int32)[0]

def move(x, y):
    cout(1)
    cout(x)
    cout(y)

def rclick(x, y):
    cout(2)
    cout(x)
    cout(y)

def key(k):
    cout(3)
    cout(k)

def getImage():
    cout(0)
    w = cin()
    h = cin()
    r = cin()
    l = cin()
    print(w, h)
    npimg = np.frombuffer(messi.stdout.read(r*h*1*4), np.uint8).reshape(h,r,4)[22:,:w,[2, 1, 0, 3]]
    return Image.fromarray(npimg)
    
def findArthas(img):
    hsv = img.convert("HSV")
    arr = np.array(hsv)
    hue = arr[:,:,0]
    sat = arr[:,:,1]
    val = arr[:,:,2]

    red_mask = (240 < hue) & (hue < 255) & (sat > 90)
    blk_mask = (val < 20)

    mask = red_mask | blk_mask

    img = (mask * 255).astype(np.uint8)

    im2, contours, hierarchy = cv2.findContours(img,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    def f(cnt):
        area = cv2.contourArea(cnt)
        x,y,w,h = cv2.boundingRect(cnt)
        rect_area = w*h
        return area > 140 and float(area)/rect_area > .6 and img[y:y+h,x:x+w].mean() > 200
    contours = list(filter(f, contours))
    # print(len(contours))
    # print(img.shape)
    if len(contours) == 1:
        x, y, w, h = cv2.boundingRect(contours[0])
        x = x + w // 2
        if x - img.shape[1] // 2 > 150:
            move(500 + x, y + 200 + 50)
            key(0)
        else:
            rclick(500 + img.shape[1] // 2 - 50, 200 + img.shape[0] // 2)
    '''   
    img = np.array(Image.fromarray(img).convert("RGB"))
    cv2.drawContours(img, contours, -1, (0,255,0), 1)
    Image.fromarray(img).show()
    '''
cout(0x3fb)
move(500, 200)
time.sleep(5)
while True:
    time.sleep(0.05)
    findArthas(getImage())
# Image.fromarray(img).show()

