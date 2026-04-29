class IInputDevices { 
public:
	virtual void init() = 0; 
	virtual void updateInputData() = 0; 
}; 

class IOutputDevices {
public: 
  virtual void init() = 0; 
  virtual void update() = 0; 
}; 
