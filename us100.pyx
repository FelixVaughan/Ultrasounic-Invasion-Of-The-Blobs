cdef extern void cleanup()
cdef extern int getDistance()
cdef extern void initialize()

class US100:
    def __init__(self):
        initialize()
        
    def __del__(self):
        cleanup()
        
    def getDeviceDistance(self):
        return getDistance()
        

        
    
