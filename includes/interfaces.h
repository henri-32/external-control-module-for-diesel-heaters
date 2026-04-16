class InputDevices { 
public:
	virtual void init() = 0; 
	virtual void updateInputData() = 0; 
}; 

class OutputDevices {
public: 
  virtual void init() = 0; 
  virtual void update() = 0; 
}; 
