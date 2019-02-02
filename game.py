from subprocess import Popen, PIPE
import numpy as np
from PIL import Image
import time
import cv2

windowX = 400
windowY = 300
windowW = 1047
windowH = 605
barW = 70
barH = 7
hanzoH = 132
hanzoR = 170

messi = Popen("./main", stdin=PIPE, stdout=PIPE)

def cout(command):
    messi.stdin.write(np.array(command, np.int32).tobytes())
    messi.stdin.flush()

def cin():
    return np.frombuffer(messi.stdout.read(4), np.int32)[0]

def mouseMove(x, y):
    cout(1)
    cout(x)
    cout(y)

def mouseRclick(x, y):
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
    return np.frombuffer(messi.stdout.read(r*h*1*4), np.uint8).reshape(h,r,4)[22:,:w,[2, 1, 0, 3]]
    
def findArthas(rgb):
    hsv = np.array(Image.fromarray(rgb).convert("HSV"))
    hue = hsv[:,:,0]
    sat = hsv[:,:,1]
    val = hsv[:,:,2]

    red_mask = ((240 < hue) | (hue < 10)) & (sat > 90)
    blk_mask = (val < 30)

    mask = red_mask | blk_mask

    img = (mask * 255).astype(np.uint8)

    im2, contours, hierarchy = cv2.findContours(img,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    def f(cnt):
        area = cv2.contourArea(cnt)
        x, y, w, h = cv2.boundingRect(cnt)
        rect_area = w * h
        if w > barW + 10 or w < barW - 10 or h > barH + 2 or h < barH - 2:
            return False
        if float(area) / rect_area < .6:
            return False
        if img[y:y+h, x:x+w].mean() < 200:
            return False
        bar = np.average(rgb[y:y+h, x:x+w, :], axis=0)
        hp = (bar[:, 0].astype(np.float) - bar[:, 1] / 2. - bar[:, 2] / 2. > 100).mean()
        if hp < .02:
            # Bardaki red orani
            return False
        return True
    contours = list(filter(f, contours))
    if 1:
        print("c", len(contours))
        for c in contours:
            x, y, w, h = cv2.boundingRect(c)
            bar = np.average(rgb[y:y+h, x:x+w, :], axis=0)
            hp = (bar[:, 0].astype(np.float) - bar[:, 1] / 2. - bar[:, 2] / 2. > 100).mean()
            print(windowX + x, windowY + y, w, h, hp)
    if 0:
        img = np.array(Image.fromarray(img).convert("RGB"))
        cv2.drawContours(img, contours, -1, (0,255,0), 1)
        Image.fromarray(img).show()
    if 1:
        def dist(c):
            x, y, w, h = cv2.boundingRect(c)
            x = x + w // 2
            y = y + h // 2
            dx = x - windowW // 2
            dy = y - (windowH // 2 - hanzoH)
            return x, y, (dx ** 2 + dy ** 2) ** .5
        contours.sort(key=lambda x: dist(x)[2])
        if len(contours) > 0:
            c = contours[0]
            x, y, d = dist(c)
            if d > hanzoR:
                mouseMove(windowX + x, windowY + y + 50)
                key(0)
            else:
                mouseRclick(windowX + windowW // 2 - 50, windowY + windowH // 2 - 27)
cout(0x3fb)
mouseMove(windowX + windowW // 2, windowY + windowH // 2 - hanzoH)
time.sleep(1)
while 1:
    # time.sleep(0.05)
    findArthas(getImage())
findArthas(getImage())
# Image.fromarray(img).show()

