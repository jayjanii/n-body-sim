### [DEMO VIDEO](https://www.youtube.com/watch?v=K6IvV7ppBJg)

[![Video Title](https://img.youtube.com/vi/K6IvV7ppBJg/0.jpg)](https://www.youtube.com/watch?v=K6IvV7ppBJg)

---

# N-Body Gravitational Simulator

Real-time N-body gravitational simulation built with OpenGL 3.3 Core Profile. Uses a symplectic Velocity Verlet integrator for long-term energy conservation. The Solar System scenario uses real J2000 Keplerian orbital elements for accurate elliptical orbits with true 3D inclinations.

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

## Building

1. Clone: `git clone https://github.com/jayjanii/n-body-sim.git`
2. Open `physics-opengl.vcxproj` in Visual Studio 2022 or later.
3. Select **Debug x64** or **Release x64**.
4. Build with `Ctrl+Shift+B`.
5. Run with `F5`. The working directory must be the project root for shaders to resolve.

> If shaders fail to load, set **Project Properties > Debugging > Working Directory** to `$(ProjectDir)`.

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

## Physics Notes

**Gravitational constant** — In AU / solar mass / year units: `G = 4π² ≈ 39.478`, derived directly from Kepler's third law.

**Velocity Verlet** — Kick-drift-kick scheme; one force evaluation per step; symplectic (bounded energy error).

```
v += 0.5 * a(t) * dt
x += v * dt
a  = computeAccelerations(x)
v += 0.5 * a * dt
```

**Keplerian state vectors** — Perifocal unit vectors Q (perihelion) and P (90° ahead) are constructed from orbital elements, then:

```
pos = r * (cos(nu)*Q + sin(nu)*P)
vel = sqrt(GM/p) * (-sin(nu)*Q + (e + cos(nu))*P)
```

True anomaly `nu` is derived from eccentric anomaly `E`, solved via Newton iteration on `M = E - e*sin(E)` to below `1e-9` rad. Astronomical ecliptic frame (Z = north) is remapped to OpenGL (Y = up) by swapping Y and Z.
