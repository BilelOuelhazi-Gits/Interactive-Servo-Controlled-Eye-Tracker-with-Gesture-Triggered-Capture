import cv2
import mediapipe as mp
import serial
import time
import threading
import os

# Initialize MediaPipe FaceMesh and Hands
mp_face_mesh = mp.solutions.face_mesh
mp_hands = mp.solutions.hands
face_mesh = mp_face_mesh.FaceMesh(max_num_faces=1, min_detection_confidence=0.7)
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)

# Initialize Serial Communication
arduino = serial.Serial('COM5', 9600)
time.sleep(2)

# Start capturing video
cap = cv2.VideoCapture(0)

# Display dimensions
display_width = 128
display_height = 64

# Timer variables
delay_time = 10
last_capture_time = 0
thumbs_up_duration = 2
thumbs_up_start_time = 0
is_thumbs_up_stable = False

# Create output directory
output_directory = "captured_images"
os.makedirs(output_directory, exist_ok=True)


def is_thumbs_up(hand_landmarks):
    thumb_tip = hand_landmarks.landmark[4]
    thumb_base = hand_landmarks.landmark[2]
    index_tip = hand_landmarks.landmark[8]
    return thumb_tip.y < index_tip.y and thumb_tip.y < thumb_base.y


def shake_servos():
    arduino.write("SHAKE\n".encode())
    time.sleep(0.01)
    arduino.write("RESET\n".encode())


def capture_picture(frame):
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    filename = os.path.join(output_directory, f'captured_image_{timestamp}.jpg')
    cv2.imwrite(filename, frame)
    print(f"Picture taken and saved as: {filename}")


frame_counter = 0  # To alternate between face and hand tracking

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # Alternate between face and hand tracking to reduce load
    if frame_counter % 2 == 0:  # Face tracking every other frame
        result_face = face_mesh.process(rgb_frame)

        if result_face.multi_face_landmarks:
            face_landmarks = result_face.multi_face_landmarks[0]
            nose_tip = face_landmarks.landmark[1]

            h, w, _ = frame.shape
            x = int(nose_tip.x * w)
            y = int(nose_tip.y * h)

            x = int((x / w) * display_width)
            y = int((y / h) * display_height)

            arduino.write(f"{x},{y}\n".encode())

    else:  # Hand tracking every other frame
        result_hands = hands.process(rgb_frame)

        if result_hands.multi_hand_landmarks:
            hand_landmarks = result_hands.multi_hand_landmarks[0]

            if is_thumbs_up(hand_landmarks):
                current_time = time.time()
                if not is_thumbs_up_stable:
                    thumbs_up_start_time = current_time
                    is_thumbs_up_stable = True
                elif current_time - thumbs_up_start_time >= thumbs_up_duration:
                    if current_time - last_capture_time >= delay_time:
                        shake_thread = threading.Thread(target=shake_servos)
                        shake_thread.start()

                        time.sleep(1)

                        arduino.write("ANIMATE\n".encode())
                        capture_thread = threading.Thread(target=capture_picture, args=(frame,))
                        capture_thread.start()

                        last_capture_time = current_time
            else:
                is_thumbs_up_stable = False

    frame_counter += 1  # Increment frame counter

    cv2.imshow("Head and Hand Tracking", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()

