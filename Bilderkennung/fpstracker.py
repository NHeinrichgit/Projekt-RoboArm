import time

class FPSTracker:
    def __init__(self):
        self.last_capture_time = time.time()
        
    def updateFPS(self):
        current_time = time.time()
        delta_time = current_time-self.last_capture_time
        fps = 1.0/delta_time if delta_time > 0 else 0
        self.last_capture_time = current_time
        return fps