'''
Author: Yinwhe
Date: 2021-12-03 14:48:30
LastEditors: Yinwhe
LastEditTime: 2021-12-03 15:32:48
Description: file information
Copyright: Copyright (c) 2021
'''

import cv2
import time
from urllib import request
from selenium import webdriver
from selenium.webdriver.common.action_chains import ActionChains
from selenium.webdriver.common.by import By
from selenium.webdriver.support.ui import WebDriverWait
from selenium.webdriver.support import expected_conditions as EC

class spider:
    def __init__(self) -> None:
        self.url = "https://www.maoyan.com/board/4"
        self.target_xpath = "//div[@id='slideBgWrap']/img"
        self.template_xpath = "//div[@id='slideBlockWrap']/img"

    def launch(self) -> None:
        self.driver = webdriver.Edge()
        self.wait = WebDriverWait(self.driver, 10, 0.5)

        self.driver.get(self.url)
        # self.driver.find_element(
        #     By.XPATH, '/html/body/div[2]/div[2]/div[1]/div/div[3]/a').click()
        # self.driver.find_element(
        #     By.XPATH, '//*[@id="loginname"]').send_keys('123456')
        # self.driver.find_element(
        #     By.XPATH, '//*[@id="nloginpwd"]').send_keys('123456')
        # self.driver.find_element(By.XPATH, '//*[@id="loginsubmit"]').click()

    def locate_pic(self, target="img1.png", template="img2.png") -> float:
        # read pic
        target_rgb = cv2.imread(target)
        target_gray = cv2.cvtColor(target_rgb, cv2.COLOR_BGR2GRAY)
        template_rgb = cv2.imread(template, 0)
        res = cv2.matchTemplate(
            target_gray, template_rgb, cv2.TM_CCOEFF_NORMED)
        value = cv2.minMaxLoc(res)
        return value[2][0]

    def solve_pic(self) -> float:
        frame = self.wait.until(
            EC.presence_of_element_located((By.ID, 'tcaptcha_iframe')))
        self.driver.switch_to.frame(frame)
        target = self.driver.find_element(By.XPATH, self.target_xpath)
        template = self.driver.find_element(By.XPATH, self.template_xpath)
        src1 = target.get_attribute("src")
        src2 = template.get_attribute("src")
        request.urlretrieve(src1, "img1.png")
        request.urlretrieve(src2, "img2.png")
        width_render = target.size['width']
        width_intrinsic = cv2.imread('img1.png').shape[1]
        return self.locate_pic() * width_render / width_intrinsic - 50

    def get_tracks(self) -> list:
        # 初速度
        v = 0
        # 计算间隔
        t = 0.1
        tracks = []
        # 当前位移
        current = 0
        # 减速阈值
        while current < self.distance:
            a = 100
            move = v * t + 0.5 * a * (t * t)
            current += move
            tracks.append(round(move))
            v = v + a * t
        tracks.append(round(self.distance + move - current))
        return tracks

    def move(self):
        self.track = self.get_tracks()
        self.slider = self.wait.until(
            EC.presence_of_element_located((By.ID, 'tcaptcha_drag_thumb')))
        ActionChains(self.driver).click_and_hold(self.slider).perform()
        for x in self.track:
            ActionChains(self.driver).move_by_offset(
                xoffset=x, yoffset=0).perform()
        ActionChains(self.driver).move_by_offset(
            xoffset=4, yoffset=0).perform()
        ActionChains(self.driver).move_by_offset(
            xoffset=-4, yoffset=0).perform()
        ActionChains(self.driver).release().perform()

    def quit(self):
        time.sleep(2)
        self.driver.quit()

    def run(self) -> None:
        self.launch()
        self.distance = self.solve_pic()
        print("Distance: {}".format(self.distance))
        self.move()
        self.quit()


if __name__ == '__main__':
    s = spider()
    s.run()
