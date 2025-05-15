#include <U8g2lib.h>
#include <Wire.h>
#include <Servo.h>

// Initialize display for SH1106 with 128×64 resolution
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

// Servo setup
Servo servoX;  // Servo for left-right movement
Servo servoY;  // Servo for up-down movement

// Eye properties
int eyeWidth = 20;      // Remove const to allow modification
int eyeHeight = 28;     // Remove const to allow modification
const int eyeXOffset = 18;    
const int blinkInterval = 5000; // Blink every 5 seconds
const int eyeColorChangeDuration = 100; // Duration for eye color change
const int excitementScale = 10; // Scale for excited eye effect

// Eye position and shape properties
int eyeXPosition = 64;  
int eyeY = 18;          

// Servo target positions for smooth movement
int targetXPosition = 90;
int targetYPosition = 90;
const int servoSpeed = 2;  // Speed of servo movement

// Timing variables
unsigned long lastBlinkTime = 0;  
bool isWandering = false;          

void setup() {
    u8g2.begin();  
    Serial.begin(9600);  
    
    // Initialize servos
    servoX.attach(7);  
    servoY.attach(6);  
    
    drawEyes();  
}

void loop() {
    handleSerialCommands();

    // Check if it's time to blink
    if (millis() - lastBlinkTime >= blinkInterval) {
        blink();  
        lastBlinkTime = millis();  
    } else {
        moveServosSmoothly();  
        if (isWandering) {
            wanderEyes();  
        } else {
            drawEyes();  
        }
    }
}

void handleSerialCommands() {
    if (Serial.available() > 0) {
        String command = Serial.readStringUntil('\n');  
        if (command == "ANIMATE") {
            animateEyes();  
        } else if (command == "SHAKE") {
            shakeServos();  
        } else {
            updateEyePosition(command);  
        }
    }
}

void updateEyePosition(String command) {
    int separatorIndex = command.indexOf(',');  
    if (separatorIndex > 0) {
        String xValue = command.substring(0, separatorIndex);
        String yValue = command.substring(separatorIndex + 1);

        eyeXPosition = constrain(xValue.toInt(), eyeWidth + eyeXOffset, u8g2.getDisplayWidth() - eyeWidth - eyeXOffset);
        eyeY = constrain(yValue.toInt(), 0, u8g2.getDisplayHeight() - eyeHeight);

        targetXPosition = map(eyeXPosition, 0, 128, 0, 180);
        targetYPosition = map(eyeY, 0, 64, 0, 180);
    }
}

void drawEyes() {
    u8g2.clearBuffer();  

    // Dynamic eye shapes
    int currentEyeWidth = eyeWidth;
    int currentEyeHeight = eyeHeight;
    if (isWandering) {
        currentEyeWidth += random(-2, 2);
        currentEyeHeight += random(-2, 2);
    }

    // Draw left eye
    u8g2.drawRBox(eyeXPosition - eyeXOffset - currentEyeWidth / 2, eyeY, currentEyeWidth, currentEyeHeight, 4);  
    u8g2.drawBox(eyeXPosition - eyeXOffset - 6, eyeY + 8, 12, 12);  

    // Draw right eye
    u8g2.drawRBox(eyeXPosition + eyeXOffset - currentEyeWidth / 2, eyeY, currentEyeWidth, currentEyeHeight, 4);  
    u8g2.drawBox(eyeXPosition + eyeXOffset - 6, eyeY + 8, 12, 12);  

    u8g2.sendBuffer();  
}

void blink() {
    // Blink with a squint effect
    for (int i = 0; i < 3; i++) {  // Repeat squinting for effect
        u8g2.clearBuffer();  

        // Draw closed eyes (horizontal lines)
        u8g2.drawHLine(eyeXPosition - eyeXOffset - eyeWidth / 2, eyeY + eyeHeight / 2, eyeWidth);  
        u8g2.drawHLine(eyeXPosition + eyeXOffset - eyeWidth / 2, eyeY + eyeHeight / 2, eyeWidth);  
        u8g2.sendBuffer();  
        delay(100);  

        drawEyes();  // Return to open eyes
        delay(100);  
    }
}

void shakeServos() {
    int currentX = servoX.read();  
    int currentY = servoY.read();  

    for (int i = 0; i < 3; i++) {  
        servoY.write(currentY + 30);   
        delay(100);      
        servoY.write(currentY - 30);   
        delay(100);  
    }

    servoX.write(currentX);
    servoY.write(currentY);
}

void animateEyes() {
    // Step 1: Rapidly expand into a thick circle
    for (int radius = 1; radius <= u8g2.getDisplayWidth() / 2; radius += 4) {  
        u8g2.clearBuffer();
        u8g2.drawCircle(u8g2.getDisplayWidth() / 2, u8g2.getDisplayHeight() / 2, radius + 20);  
        u8g2.sendBuffer();
        delay(10);  
    }

    // Step 2: Flash effect with fade-in and fade-out
    for (int brightness = 0; brightness <= 255; brightness += 51) {  
        u8g2.setContrast(brightness);
        u8g2.sendBuffer();
        delay(eyeColorChangeDuration);  
    }

    u8g2.setContrast(255);
    u8g2.sendBuffer();
    delay(100);  

    for (int brightness = 255; brightness >= 0; brightness -= 51) {  
        u8g2.setContrast(brightness);
        u8g2.sendBuffer();
        delay(eyeColorChangeDuration);  
    }

    // Step 3: Fast return to normal
    for (int radius = u8g2.getDisplayWidth() / 2; radius >= 1; radius -= 4) {  
        u8g2.clearBuffer();
        u8g2.drawCircle(u8g2.getDisplayWidth() / 2, u8g2.getDisplayHeight() / 2, radius + 20);  
        u8g2.sendBuffer();
        delay(10);  
    }

    u8g2.setContrast(255);  
    drawEyes(); 
}

// Function to smoothly move the servos towards target positions
void moveServosSmoothly() {
    int currentX = servoX.read();
    int currentY = servoY.read();

    // Move X servo
    currentX += (targetXPosition - currentX) > 0 ? servoSpeed : -servoSpeed;
    servoX.write(constrain(currentX, 0, 180));  

    // Move Y servo
    currentY += (targetYPosition - currentY) > 0 ? servoSpeed : -servoSpeed;
    servoY.write(constrain(currentY, 0, 180));  
}

// Function to add wandering behavior for the eyes
void wanderEyes() {
    if (millis() % 2000 < 100) {  
        int wanderX = random(-10, 11);  
        int wanderY = random(-5, 6);  
        eyeXPosition += wanderX;  
        eyeY += wanderY;  

        eyeXPosition = constrain(eyeXPosition, eyeWidth + eyeXOffset, u8g2.getDisplayWidth() - eyeWidth - eyeXOffset);
        eyeY = constrain(eyeY, 0, u8g2.getDisplayHeight() - eyeHeight);
        
        targetXPosition = map(eyeXPosition, 0, 128, 0, 180);
        targetYPosition = map(eyeY, 0, 64, 0, 180);

        isWandering = true;

        drawEyes();
    }
}

// Function to create excitement with enlarging eyes
void excitedBlink() {
    // Enlarge eyes
    int originalWidth = eyeWidth;
    int originalHeight = eyeHeight;

    for (int scale = 0; scale <= excitementScale; scale++) {
        int newWidth = originalWidth + scale; // Use a temporary variable
        int newHeight = originalHeight + scale / 2; // Use a temporary variable

        // Ensure the new sizes do not exceed certain limits
        newWidth = constrain(newWidth, 10, 50);  
        newHeight = constrain(newHeight, 10, 50);  

        // Draw eyes with updated sizes
        u8g2.clearBuffer();  
        u8g2.drawRBox(eyeXPosition - eyeXOffset - newWidth / 2, eyeY, newWidth, newHeight, 4);  
        u8g2.drawBox(eyeXPosition - eyeXOffset - 6, eyeY + 8, 12, 12);  
        u8g2.drawRBox(eyeXPosition + eyeXOffset - newWidth / 2, eyeY, newWidth, newHeight, 4);  
        u8g2.drawBox(eyeXPosition + eyeXOffset - 6, eyeY + 8, 12, 12);  
        
        u8g2.sendBuffer();  
        delay(50);  
    }

    // Reset eyes
    eyeWidth = originalWidth;  
    eyeHeight = originalHeight;  
    drawEyes();  
}
