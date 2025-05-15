# Interactive-Servo-Controlled-Eye-Tracker-with-Gesture-Triggered-Capture
This project combines computer vision with servo-controlled animatronic eyes and serial communication to create an interactive system. Using MediaPipe, the system tracks a user's face and hand gestures via webcam input. The position of the user's nose controls the movement of servo-driven eyes displayed on an SH1106 OLED screen, while a detected "thumbs up" gesture triggers a sequence that shakes the servos, animates the eyes, and captures a photo.

Key Features:

Real-time face and hand tracking using MediaPipe.

Smooth servo movement controlling animatronic eyes on a 128x64 SH1106 display.

Gesture detection (thumbs up) to trigger eye animation and photo capture.

Multithreaded Python script for efficient hardware interaction and image saving.

Arduino firmware handles servo control, eye animations, blinking, wandering behavior, and reacts to serial commands.

This integration showcases interactive robotics and computer vision working together for a responsive, playful user experience.
