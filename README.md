---

# N-Body Gravitational Simulator

Real-time N-body gravitational simulation built with OpenGL 3.3 Core Profile. Uses a symplectic Velocity Verlet integrator for long-term energy conservation. The Solar System scenario uses real J2000 Keplerian orbital elements for accurate elliptical orbits with true 3D inclinations.

### [DEMO VIDEO LINK](https://www.youtube.com/watch?v=K6IvV7ppBJg)

<p align="center">
    <img src="demo-assets/demo1.gif" width="400" alt="demo1">
    <img src="demo-assets/demo2.gif" width="400" alt="demo2">
</p>

---

## Features

- **N-body gravity** — All pairs computed each frame using Newton's third law (one evaluation per pair).
- **Velocity Verlet (kick-drift-kick)** — Second-order symplectic integrator. Stable elliptical orbits with bounded energy error.
- **Keplerian initial conditions** — Solar System planets initialised from J2000 orbital elements (a, e, i, Ω, ω, M₀) via Newton-iterated Kepler's equation.
- **Spacetime curvature grid** — Dynamic Flamm's paraboloid mesh that deforms under massive bodies. Fully runtime-configurable.
- **Orbit trails** — Per-body ring-buffer trail rendered as a line strip.
- **Star glow billboard** — Camera-facing quad with Gaussian falloff and additive blending per star.
- **Phong shading** — Diffuse and ambient lighting sourced from all stars in the scene.
- **Visual body scale** — Global render radius multiplier with no effect on physics.
- **Scenario system** — Eight presets selectable at runtime from a dropdown; switching resets all state instantly.

---

## Scenarios

The simulator includes a variety of pre-configured orbital scenarios ranging from real-world astronomical data to theoretical mathematical alignments:

- **Solar System (9 bodies)**: Features the Sun and 8 planets initialized from real J2000 Keplerian elements, providing accurate elliptical orbits and true 3D inclinations.
- **Binary Stars (3 bodies)**: Models two stars of unequal mass orbiting their shared barycenter, accompanied by a distant circumbinary planet.
- **Earth + Moon + Sun (3 bodies)**: A demonstration scale where Earth's mass is artificially increased to maintain a stable and visually discernible lunar orbit.
- **Symmetric 3-Body V1 (3 bodies)**: Three equal masses positioned on an equilateral triangle, utilizing analytically derived tangential speeds for stable orbits.
- **Symmetric 3-Body V2 (3 bodies)**: The classic Sitnikov problem, featuring a binary pair orbiting in the XZ plane while a test particle oscillates vertically along the Y-axis.
- **Symmetric 4-Body V1 (4 bodies)**: Four equal masses locked in a stable orbital square configuration.
- **Symmetric 4-Body V2 (4 bodies)**: An unstable arrangement of two orthogonal binary pairs orbiting in mutually perpendicular planes.
- **Symmetric 5-Body (5 bodies)**: Five equal masses maintaining stability along a regular pentagon.

---

## Controls

### Camera Navigation
Navigate the 3D space using standard free-look controls. 
- Use **W / S** to move forward and backward, and **A / D** to strafe sideto-side. 
- Ascend and descend using **Space** and **Left Ctrl**. 
- To look around, simply **Left Click and Drag** the mouse. 
- For faster travel, hold **Left Shift** to apply a 5x speed multiplier. You can also permanently adjust your base movement speed by scrolling the **Mouse Wheel** to speed up or slow down.

### Simulation Panel
An interactive Dear ImGui interface allows you to tweak the environment and visual parameters on the fly:
- **Environment**: Instantly load a new scene from the **Scenario** dropdown or adjust the **Time Scale** (from paused up to 5 simulated years per real second).
- **Visuals**: Change the **Visual Body Scale** to make celestial bodies larger without impacting physics, or modify the **Trail Length** to alter how much orbital history is drawn.
- **Spacetime Grid**: Toggle the visual grid on or off. You can fine-tune its deeper characteristics, including the **Curvature** intensity under massive bodies, its overall physical **Size**, the vertical **Grid Base Y** offset, and its underlying **Resolution**, which dynamically rebuilds the GPU buffers.

---

## Requirements & Dependencies

To build and run the simulation, you will need a system running **Windows 10 x64 (or later)** alongside a GPU that supports the **OpenGL 3.3 Core Profile**. The development environment requires **Visual Studio 2022 (toolset v145 or later)** using C++20 and the Windows 10 SDK.

**No complicated installations are required** for external libraries, as they are vendored directly in the repository:
- **GLFW (3.4)** handles window creation, contexts, and user input.
- **GLAD (0.1.36)** is used for loading OpenGL core functions.
- **GLM (1.0.3)** provides the essential vector and matrix mathematics.
- **Dear ImGui (1.92.7)** powers the interactive runtime control panel.

---

## Project Structure

```
physics-opengl/
├── src/
│   ├── main.cpp            Render loop, Velocity Verlet integrator
│   ├── Camera.cpp          Free-look camera
│   ├── Scenarios.cpp       Scenario definitions, Keplerian solver
│   ├── Sphere.cpp          Sphere mesh and draw
│   ├── Object.cpp          Base physics object
│   ├── Grid.cpp            Spacetime curvature mesh
│   ├── OrbitTrail.cpp      Ring-buffer orbit trail
│   ├── Shader.cpp          GLSL compilation and linking
│   ├── VAO/VBO/EBO.cpp     GPU buffer wrappers
│   └── imgui/              Dear ImGui backend
├── dependencies/
│   ├── include/            GLAD, GLFW, GLM, project headers
│   ├── imgui/              Dear ImGui headers
│   └── lib/glfw3.lib       Precompiled GLFW (x64)
└── glsl shaders/
    ├── default.vert/.frag  Body shader (Phong + emissive)
    ├── grid.vert/.frag     Grid and trail shader
    └── glow.vert/.frag     Star glow billboard
```

---

## Physics Implementation

### Numerical Integration
To maintain orbital stability and conserve energy, this simulation utilizes a **Velocity Verlet** (kick-drift-kick) scheme. This is a symplectic integrator, meaning it is specifically designed to keep the energy error bounded over long durations.

$$
\begin{aligned}
v &\leftarrow v + 0.5 \cdot a(t) \cdot dt \\
x &\leftarrow x + v \cdot dt \\
a &\leftarrow \text{computeAccelerations}(x) \\
v &\leftarrow v + 0.5 \cdot a \cdot dt
\end{aligned}
$$

### Keplerian State Vectors
The simulation constructs orbits using perifocal unit vectors $\mathbf{Q}$ (perihelion) and $\mathbf{P}$ ($90^\circ$ ahead). 

$$
\begin{aligned}
\mathbf{pos} &= r \cdot (\cos(\nu)\mathbf{Q} + \sin(\nu)\mathbf{P}) \\
\mathbf{vel} &= \sqrt{\frac{GM}{p}} \cdot (-\sin(\nu)\mathbf{Q} + (e + \cos(\nu))\mathbf{P})
\end{aligned}
$$

> **Note:** The true anomaly $\nu$ is derived from the eccentric anomaly $E$, solved via Newton iteration on Kepler's Equation $M = E - e \cdot \sin(E)$ to a precision below $1e^{-9}$ rad.

---

## Orbital Configurations

### Orthogonal Intersecting Binaries
This setup places two binary pairs on planes exactly $90^\circ$ apart (e.g., $XZ$ and $XY$ planes).

* **A Pure Math Creation:** Real star systems usually form on a single flat plane. These are a pure mathematical concept possible only in code.
* **The Ejection Inevitability:** Because the planes are orthogonal, their forces cancel out perfectly. If the symmetry is broken by even a tiny fraction, the system becomes chaotic, often resulting in one pair being ejected into deep space.

### The Sitnikov Problem
A classic 3D symmetric setup where two massive bodies orbit in a plane while a third "oscillator" body moves strictly along the vertical $Z$-axis, passing through the center of mass.