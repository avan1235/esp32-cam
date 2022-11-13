#!/usr/bin/env python3

import cv2

from camera import Camera

WINDOW_NAME = "demo"


def main():
    cv2.namedWindow(WINDOW_NAME)
    cv2.startWindowThread()
    with Camera() as cam:
        while True:
            img = cam.get_frame()
            if img is None:
                break
            cv2.imshow(WINDOW_NAME, img)
            keypress = cv2.pollKey() & 0xFF
            if keypress == ord("q"):
                break
            elif keypress == ord("+"):
                cam.increase_quality()
            elif keypress == ord("-"):
                cam.decrease_quality()
            elif keypress == ord("l"):
                cam.switch_flash_led()
    cv2.destroyWindow(WINDOW_NAME)


if __name__ == "__main__":
    main()
