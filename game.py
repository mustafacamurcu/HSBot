from subprocess import Popen, PIPE
import numpy as np
from PIL import Image
import colorsys
import time
import cv2
import random
import requests

windowX = 400
windowY = 300
windowW = 1047
windowH = 605
vallaX = windowW // 2
vallaY = windowH // 2 - 30
barW = 68
barH = 6
barY = 80
vallaR = 150 #daha yuksek daha korkak
qR = 250
eR = 80
wR = 250
rR = 150

state = "attack"

messi = Popen("./main", stdin=PIPE, stdout=PIPE)

def cout(command):
    messi.stdin.write((str(command) + "\n").encode())
    messi.stdin.flush()

def cin():
    return np.frombuffer(messi.stdout.read(4), np.int32)[0]

def Q(x, y):
    mouseMove(x, y)
    key(0xC)

def W(x, y):
    mouseMove(x, y)
    key(0xD)
    
def E(x, y):
    mouseMove(x, y)
    key(0xE)

def R(x, y):
    mouseMove(x, y)
    key(0xF)

def Attack(x, y):
    mouseMove(x, y)
    key(0)

def Move(x, y):
    x = min(max(x, 30), windowW - 30)
    y = min(max(y, 30), windowH - 30)
    cout(2)
    cout(int(x + windowX))
    cout(int(y + windowY))

def Explore(dt):
    pChange = 1
    global lastdx, lastdy
    if random.uniform(0,pChange) < dt:
        dX = random.gauss(windowW/10, windowW/10)
        dY = random.uniform(-windowW/10, windowW/10)
        lastdx = max(-windowW/10, min(windowW/10, dX))
        lastdy = max(-windowW/10, min(windowW/10, dY))
    Move(vallaX + lastdx, vallaY + lastdy)

def mouseMove(x, y):
    x = min(max(x, 30), windowW - 30)
    y = min(max(y, 30), windowH - 30)
    cout(1)
    cout(int(x + windowX))
    cout(int(y + windowY))

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

def isHP(h,s,v):
    return ((240 < h) or (h < 10)) and (s > 150) and (v > 130)

def isHealing(h,s,v):
    return ((240 < h) or (h < 10)) and (s > 150) and (v < 120) and (v > 80)

def isBlack(h,s,v):
    return (v < 70)

def findEnemyHeroes(rgb):
    hsv = np.array(Image.fromarray(rgb).convert("HSV"))
    hue = hsv[:,:,0]
    sat = hsv[:,:,1]
    val = hsv[:,:,2]

    HUE_MIN = 240
    HUE_MAX = 10
    SAT_MIN = 150
    VAL_MIN = 130

    mask = ((HUE_MIN < hue) | (hue < HUE_MAX)) & (sat > SAT_MIN) & (val > VAL_MIN)

    img = (mask * 255).astype(np.uint8)

    im2, contours, hierarchy = cv2.findContours(img,cv2.RETR_TREE,cv2.CHAIN_APPROX_SIMPLE)
    def f(cnt):
        area = cv2.contourArea(cnt)
        x, y, w, h = cv2.boundingRect(cnt)
        rect_area = (w-1) * (h-1)
        if abs(x - vallaX) < 320 and y < 60:
            # top bar
            return False
        if x > 800 and y > 360:
            # minimap
            return False
        if abs(x - vallaX) < 200 and y > 520:
            # skills
            return False
        if h > barH + 1 or h < barH -1:
            return False
        if float(area) < rect_area * .7:
            # print("rect filter", x, y, cv2.boundingRect(cnt), area)
            return False

        if x + barW > windowW or y + barH > windowH:
            return False
        if barW - w > barH:
            bar_rest = rgb[y:y+barH, x+w:x+barW, :].astype(np.float)
            bar_rest = np.average(bar_rest, axis=0)
            ctr = 0
            for i in range(bar_rest.shape[0]):
                h, s, v = colorsys.rgb_to_hsv(*bar_rest[i,:3] / 255)
                h *= 255
                s *= 255
                v *= 255
                if isBlack(h,s,v) or isHP(h,s,v) or isHealing(h,s,v):
                    ctr += 1
            if float(ctr) / bar_rest.shape[0] < .8:
                # print("rest_filter", x, y, float(ctr) / bar_rest.shape[0])
                return False
        return True
    contours = list(filter(f, contours))
    # Remove overlapping
    i = 0
    while i < len(contours):
        p = contours[i]
        for j, q in enumerate(contours):
            if i != j:
                x1, y1, _, _ = cv2.boundingRect(p)
                x2, y2, _, _ = cv2.boundingRect(q)
                if abs(y1 - y2) < 2 and x1 - x2 < barW and x1 > x2:
                    del contours[i]
                    i -= 1
                    break
        i += 1
    if 0:
        img = np.array(Image.fromarray(img).convert("RGB"))
        cv2.drawContours(img, contours, -1, (0,255,0), 1)
        Image.fromarray(img).show()
        # Image.fromarray(rgb).show()
    pos = []
    for c in contours:
        x, y, w, h = cv2.boundingRect(c)
        x = x + barW // 2 - (x - vallaX) // 8
        y = y + barH // 2 + barY
        pos.append((x, y))
    return pos

lastdx = 0
lastdy = 0
def AI(dt):
    global state
    m = checkMessage()
    if m:
        print(m)
        state = m
    rgb = getImage()
    pos = findEnemyHeroes(rgb)
    cvpic = rgb[::3,::3,[2, 1, 0]]
    a = 2
    for x, y in pos:
        x //= 3
        y //= 3
        cvpic[y-a:y+a,x-a:x+a,1] = 255
    x = vallaX // 3
    y = vallaY // 3
    cvpic[y-a:y+a,x-a:x+a,1] = 255
    cv2.imshow('prev', cvpic)
    cv2.waitKey(1)
    #print(len(pos))
    if len(pos) > 0:
        def dist(p):
            x, y = p
            dx = x - vallaX
            dy = y - vallaY
            return (dx ** 2 + dy ** 2) ** .5
        pos.sort(key=dist)
        print(pos)
        print("---------------------")
        x, y = pos[0]
        d = dist(pos[0])
        if d < qR: # Q
            Q(x, y)
        if d < wR: # W
            W(x, y)
        if d < rR: # R
            R(x, y)
        if state == 'attack':
            Attack(x, y)
            if d > 2 * vallaR and d < 4 * vallaR: # chase with E
                E(x, y)
        elif state == 'kite':
            if d > vallaR: # attack 
                Attack(x, y)
            else: 
                Move(vallaX - 200, vallaY) # move back
        elif state == 'retreat':
            Move(2 * vallaX - x, 2 * vallaY - y) # move back
            if d < eR: # run away with E
                E(2 * vallaX - x, 2 * vallaY - y)
        else:
            Explore(dt)
    else:
        Explore(dt)

lastMessage = 0
def checkMessage():
    r = requests.get('http://akkas.scripts.mit.edu/HSBot/comm.html')
    t, m = r.text.strip().split(" ")
    t = int(t)
    if t > lastMessage:
        return m
    else:
        return None

def unittests():
    assert set(findEnemyHeroes(np.array(Image.open("tests/p1.png")))) == set([(796, 335), (618, 236)])

cout(0x10ae)
#unittests()
mouseMove(0, 0)
time.sleep(0)
cv2.namedWindow('prev')
cv2.moveWindow('prev', -50, 650)
lastTime = time.time()
while 1:
    dt = time.time() - lastTime
    # time.sleep(0.05)
    AI(dt)
    lastTime += dt

if 0:
    findEnemyHeroes(np.array(Image.open("tests/p3.png")))
#image = getImage()
#Image.fromarray(image).save("game_image.png")
#findEnemyHeroes(image)

# TODO
# - Exploration
# - Shield and PoisonHP
# - Minions
# - Avoid Turrets
