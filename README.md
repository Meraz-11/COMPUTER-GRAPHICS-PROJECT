# Visualization of Industrial Waste–Driven River Pollution in Bangladesh

This project is a Computer Graphics simulation built using **C++ and OpenGL (GLUT)** that visualizes how industrial waste pollutes rivers in Bangladesh and demonstrates the transition from a polluted environment to a cleaner one through animation and interaction.

The goal of this project is to raise awareness about environmental pollution while applying real-time graphics concepts such as transformations, animation, and interaction.

---

## 🎯 Objectives

- Visualize industrial waste discharge into rivers.  
- Simulate pollution effects such as smoke, trash, dead fish, and toxic water.  
- Demonstrate environmental recovery through cleaning animation.  
- Apply computer graphics concepts in a real-life problem context.  

---

## 🛠 Technologies Used

- C++  
- OpenGL  
- GLUT / FreeGLUT  
- CodeBlocks / Visual Studio  

---

## ⚙ Setup Instructions

1. Install **CodeBlocks with MinGW** or Visual Studio.  
2. Download and configure **FreeGLUT**.  
3. Copy the following files into your system folders:

```
freeglut.dll  → C:\Windows\System32  
freeglut.h    → compiler\include\GL  
libfreeglut.a → compiler\lib  
```

4. Create a new **Console Application (C++)** project.  
5. Add the `.cpp` source file to the project.  
6. Add these libraries to linker settings:

```
-lopengl32  
-lglu32  
-lfreeglut  
```

7. Build and Run ▶

---

## 🎮 Control Instructions

| Key | Action |
|-----|--------|
| `S` | Start Pollution Animation |
| `B` | boat appears and moves|
| `C` | Clean the Environment and water|
| `M` |  truck comes from left and parks|
| `Q` | air cleans + smoke disappears + trees & factory land turn green + truck leaves |
| `ESC` | Exit Program |

---

## 🌱 Features

- Industrial factory with smoke animation  
- Toxic waste pipe to river  
- Polluted water color transition  
- Floating trash movement  
- Dead fish visualization  
- Cloud and bird animation  
- Interactive cleaning effect  


---

## 🚀 Future Improvements

- Add rain and water ripple effects.  
- Implement sound effects.  
- Include GUI controls.  
- Simulate real-time pollution data.  

---




