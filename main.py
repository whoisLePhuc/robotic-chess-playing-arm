import cv2
import vision_manager.vision_module as vm
import os

cbPattern = cv2.imread("reference_image/cb_pattern.jpg", cv2.IMREAD_GRAYSCALE)

cap = cv2.VideoCapture(0)

if not cap.isOpened():
    print("Không thể mở camera")
    exit()


_, img = cap.read()
retIMG, H = vm.findTransformation(img, cbPattern)

while True:
    # Đọc frame từ camera
    ret, img = cap.read()
    if not ret:
        print("Không nhận được frame")
        break
    
    imgAligned = vm.applyHomography(img, H)
    imgQuad = vm.drawQuadrants(imgAligned)
    
    cv2.imshow("Chessboard", imgQuad)

    # Nhấn 'q' để thoát
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Giải phóng tài nguyên
cap.release()
cv2.destroyAllWindows()
