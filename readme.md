# NeuroScan

**NeuroScan** is an experimental brain visualization and neural signal analysis platform built with **C++20, Vulkan, GLSL and Qt Quick**.

The project explores how biological brain signals could be acquired, processed and represented on an interactive 3D anatomical model of the human brain.

The long-term objective is to connect **external stimuli, biosignal acquisition, signal processing and real-time 3D visualization** within a single software platform.

> NeuroScan is currently under active development and is not a medical device.

---

## Project Vision

NeuroScan was originally conceived as more than a 3D brain viewer.

The idea is to build an experimental platform capable of combining measurements of brain activity with a detailed digital representation of the human brain.

One of the main concepts explored by the project is the brain's response to an external stimulus.

For example, when a person hears a spoken word or a sound:

```text
Spoken Word / Sound
        │
        ▼
Auditory Stimulus
        │
        ▼
Biological Response
        │
        ▼
EEG / Sensors
        │
        ▼
Signal Acquisition
        │
        ▼
Signal Processing & Analysis
        │
        ▼
Brain Region Estimation
        │
        ▼
NeuroScan Visualization Engine
        │
        ▼
Interactive 3D Activity Visualization
```

The objective is to investigate how measurable biosignals can be processed and associated with anatomical brain regions, allowing changes in measured activity to be visualized on a 3D brain model.

NeuroScan does **not** assume that technologies such as scalp EEG can directly observe individual neurons or perfectly reconstruct the propagation of neural signals.

Instead, the project aims to explore what information can realistically be extracted from available sensors and how that information can be represented visually.

---

## Current Development

The project is currently focused on building the **visualization engine and application architecture** required for the future signal-processing pipeline.

The current implementation already includes a functional Vulkan-based 3D renderer capable of displaying a detailed anatomical brain model.

Current development areas include:

- Real-time 3D brain rendering
- Anatomical GLB model loading
- Interactive camera controls
- Custom GLSL brain shaders
- Procedural neural activity visualization
- Adjustable rendering parameters
- Qt Quick / QML scientific interface
- Modular C++ rendering architecture

The neural activity currently displayed by the shaders is **simulated** and is not derived from real neurological measurements.

---

## Tech Stack

### Core

- **C++20**
- **CMake**

### Graphics

- **Vulkan**
- **GLSL**
- **GLM**
- **Assimp**

### Interface

- **Qt 6**
- **Qt Quick**
- **QML**

### Data & Models

- GLB / glTF anatomical models

Future versions of the project may introduce dedicated signal-processing and machine-learning components.

---

## 3D Visualization Engine

NeuroScan uses Vulkan as its primary rendering API.

The renderer currently supports:

- Vertex and index buffers
- Depth testing
- Alpha blending
- Dynamic viewport and scissor
- Perspective projection
- Anatomical mesh normalization
- Surface lighting
- Fresnel effects
- Procedural activity effects
- GPU-driven visualization through GLSL shaders

A detailed anatomical brain model can be loaded using Assimp and rendered directly by the Vulkan pipeline.

---

## Neural Activity Visualization

The current fragment shader contains procedural activity regions designed to prototype how neural activity could eventually be represented.

The visualization includes:

- Animated activity hotspots
- Pulsating regions
- Surface illumination
- Electric blue and cyan activity
- High-activity color transitions
- Procedural signal variations

At the moment, these signals are generated mathematically by the shader.

```text
CURRENT

Procedural Simulation
        │
        ▼
GLSL Activity Model
        │
        ▼
Vulkan Renderer
        │
        ▼
3D Brain
```

The long-term architecture is intended to evolve toward:

```text
FUTURE

EEG / Biosensors
        │
        ▼
Signal Acquisition
        │
        ▼
Filtering & Processing
        │
        ▼
Signal Analysis
        │
        ▼
Source / Region Estimation
        │
        ▼
Visualization Data
        │
        ▼
Vulkan Renderer
        │
        ▼
Interactive 3D Brain
```

This separation allows the visualization engine to be developed before physical sensors and signal-processing systems are introduced.

---

## Interactive Camera

The current renderer includes an orbital camera system for exploring the anatomical model.

It supports:

- Brain rotation
- Zoom
- Perspective projection
- Viewport-based interaction

Future versions are planned to provide dedicated anatomical viewpoints and region-focused navigation.

---

## User Interface

NeuroScan is transitioning to a **Qt Quick / QML interface** while keeping the performance-critical rendering engine in C++ and Vulkan.

The interface is designed around a scientific dashboard containing components such as:

- Brain visualization viewport
- Anatomical layer controls
- Transparency controls
- Brightness controls
- Activity visualization controls
- Brain region information
- Activity monitoring
- Time controls
- Neuron inspection
- Session information

The goal is to keep the rendering engine and interface separated so that both systems can evolve independently.

---

## Architecture

```text
NeuroScan
│
├── User Interface
│   └── Qt Quick / QML
│
├── Visualization Engine
│   ├── C++20
│   ├── Vulkan
│   └── GLSL
│
├── Anatomical Data
│   ├── GLB / glTF
│   └── Brain regions
│
├── Signal Processing            [Planned]
│   ├── Filtering
│   ├── Frequency analysis
│   ├── Artifact handling
│   └── Feature extraction
│
└── Biosignal Acquisition        [Planned]
    ├── EEG
    └── Other compatible sensors
```

Current source structure:

```text
NeuroScan/
│
├── assets/
│   └── models/
│       └── brain.glb
│
├── qml/
│   ├── Main.qml
│   └── components/
│
├── shaders/
│   ├── brain.vert
│   └── brain.frag
│
├── src/
│   ├── main.cpp
│   │
│   ├── qt/
│   │   └── BrainViewport
│   │
│   └── renderer/
│       ├── BrainMesh
│       └── Camera
│
├── CMakeLists.txt
├── .gitignore
└── README.md
```

---

## Roadmap

### Phase 1 — Visualization Engine

- [x] Vulkan initialization
- [x] Vulkan rendering pipeline
- [x] Anatomical GLB brain loading
- [x] GPU brain rendering
- [x] Depth testing
- [x] Interactive orbital camera
- [x] Custom GLSL brain shaders
- [x] Procedural activity visualization

### Phase 2 — Application Interface

- [x] Initial visualization controls
- [ ] Complete Qt Quick interface
- [ ] Integrate Vulkan rendering into the Qt Quick viewport
- [ ] Anatomical layer controls
- [ ] Brain region selection
- [ ] Region metadata
- [ ] Activity heatmaps
- [ ] Session controls
- [ ] Recording and playback

### Phase 3 — Brain Data

- [ ] Anatomical brain region mapping
- [ ] Coordinate system for activity data
- [ ] Brain atlas integration
- [ ] Region-based activity representation
- [ ] Time-dependent activity visualization

### Phase 4 — Biosignal Processing

- [ ] EEG acquisition
- [ ] Signal filtering
- [ ] Artifact handling
- [ ] Frequency-band analysis
- [ ] Signal feature extraction
- [ ] Brain-region estimation
- [ ] Real-time visualization pipeline

### Phase 5 — Experimental Analysis

- [ ] Stimulus synchronization
- [ ] Auditory experiment support
- [ ] Event markers
- [ ] Session comparison
- [ ] Activity pattern analysis
- [ ] Experimental ML models

---

## Experimental Auditory Pipeline

One of the long-term experiments envisioned for NeuroScan involves studying responses to auditory stimuli.

A simplified experimental pipeline could be:

```text
              AUDITORY EXPERIMENT

                    Sound
                      │
                      ▼
              ┌───────────────┐
              │   Participant │
              └───────┬───────┘
                      │
                      ▼
               Brain Response
                      │
                      ▼
              ┌───────────────┐
              │ EEG / Sensors │
              └───────┬───────┘
                      │
                      ▼
              Signal Acquisition
                      │
                      ▼
           Filtering / Processing
                      │
                      ▼
             Activity Estimation
                      │
                      ▼
              Anatomical Mapping
                      │
                      ▼
              ┌───────────────┐
              │   NeuroScan   │
              └───────┬───────┘
                      │
                      ▼
             Interactive 3D View
```

The purpose would be to compare measured brain activity before, during and after controlled stimuli and visualize the resulting data spatially and temporally.

---

## Building

### Requirements

NeuroScan currently requires:

- CMake 3.24+
- C++20 compatible compiler
- Vulkan
- Qt 6
- GLM
- Assimp

### macOS

The project is currently primarily developed on **Apple Silicon**.

Dependencies can be installed using Homebrew:

```bash
brew install cmake
brew install qt
brew install glm
brew install assimp
brew install vulkan-loader
```

Configure the project:

```bash
cmake -S . -B build \
    -DCMAKE_PREFIX_PATH="$(brew --prefix qt)"
```

Compile:

```bash
cmake --build build -j
```

Run:

```bash
./build/NeuroScan
```

Depending on the Vulkan installation, additional MoltenVK or Vulkan SDK configuration may be required on macOS.

---

## Scientific Scope

NeuroScan is primarily a **software engineering, graphics and experimental neuroscience visualization project**.

The project makes an important distinction between:

```text
Measured signal
      ≠
Exact neural activity
      ≠
Individual neuron activity
```

Signals recorded outside the brain are indirect measurements and have physical and spatial limitations.

For this reason, future NeuroScan features involving EEG or other biosignals will distinguish between:

- Raw measurements
- Processed signals
- Estimated activity
- Simulated activity
- Anatomical information

This distinction is important for keeping visualizations scientifically interpretable.

---

## Project Status

**Experimental — Active Development**

The core Vulkan brain renderer is functional.

The project is currently transitioning from the original development interface toward a Qt Quick-based application architecture.

Real biosignal acquisition and EEG-based activity visualization are **planned research and development features and are not currently implemented**.

---

## Disclaimer

NeuroScan is an experimental software engineering and visualization project.

It is **not a medical device**, does not provide medical diagnoses and should not be used for clinical decision-making.

Any future brain-activity visualization derived from biosignals should be interpreted according to the limitations of the acquisition and signal-processing methods used.

---

## Author

Developed by **Kilian Berardino**.
