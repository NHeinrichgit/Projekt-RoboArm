import cv2 as cv

class ScreenText:
    def __init__(self, text=None, position=None):
        if text is None:
            text = "Default Text"
        if position is None:
            position = (0,0)
        self.text = text
        self.position = position  # Position for the text (x, y)
    font = cv.FONT_HERSHEY_SIMPLEX  # Font type
    font_scale = 0.7  # Font size
    color = (0, 255, 0)  # Green text (in BGR format)
    thickness = 1  # Thickness of the text
    line_type = cv.LINE_AA  # Anti-aliased line type
    
    def set_Text(self, text):
        self.text=text
    
    def set_Position(self, position):
        self.position=position
        
    def set_Color(self, color):
        self.color = color
    
    def showText(self, frame):
        return cv.putText(frame, self.text, self.position, self.font, self.font_scale, self.color, self.thickness, self.line_type)