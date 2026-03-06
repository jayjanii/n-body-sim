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

| Scenario | Bodies | Description |
|---|---|---|
| Solar System | 9 | Sun + 8 planets from J2000 Keplerian elements. Elliptical orbits, real inclinations. |
| Binary Stars | 3 | Two unequal stars orbiting their barycenter with a circumbinary planet. |
| Earth + Moon + Sun | 3 | Earth's mass scaled up to keep the Moon's orbit stable at a visible radius. |
| Symmetric 3-Body V1 | 3 | Equal masses on an equilateral triangle with analytically derived tangential speeds. |
| Symmetric 3-Body V2 | 3 | Sitnikov problem — binary in XZ plane, test particle oscillating on Y-axis. |
| Symmetric 4-Body V1 | 4 | Equal masses on a square. |
| Symmetric 4-Body V2 | 4 | Two orthogonal binary pairs orbiting in perpendicular planes. |
| Symmetric 5-Body | 5 | Equal masses on a regular pentagon. |

---

## Controls

### Camera

| Input | Action |
|---|---|
| `W` / `S` | Move forward / backward |
| `A` / `D` | Strafe left / right |
| `Space` / `Left Ctrl` | Move up / down |
| `Left Shift` (hold) | 5x speed multiplier |
| Scroll Wheel | Increase or decrease base speed by 25% / 20% (persistent, clamped to `[0.0001, 20.0]`) |
| Left Mouse Button (hold + drag) | Rotate camera |

### Simulation Panel

| Control | Range | Description |
|---|---|---|
| Scenario | Dropdown | Load a preset |
| Time Scale | 0.0 – 5.0 yr/s | Simulated years per real second |
| Visual Body Scale | 0.01 – 5.0 | Render radius multiplier |
| Curvature | 0.0 – 3.0 | Grid deformation depth |
| Trail Length | 0 – 2000 | Max trail points per body |
| Show Spacetime Grid | Checkbox | Toggle grid |
| Grid Size | 1.0 – 100.0 AU | Grid half-extent |
| Grid Base Y | -5.0 – 5.0 | Grid vertical offset |
| Grid Resolution | 10 – 200 | Divisions per axis (rebuilds GPU buffers on change) |

---

## Requirements

| Requirement | Version |
|---|---|
| OS | Windows 10 x64 or later |
| GPU | OpenGL 3.3 Core Profile |
| Visual Studio | 2022 (toolset v145) or later |
| Windows SDK | 10.0 |
| C++ standard | C++20 |

### Dependencies (vendored, no install required)

| Library | Version | Purpose |
|---|---|---|
| GLFW | 3.4 | Window, context, input |
| GLAD | 0.1.36 | OpenGL function loader |
| GLM | 1.0.3 | Vector and matrix math |
| Dear ImGui | 1.92.7 | Runtime control panel |

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
