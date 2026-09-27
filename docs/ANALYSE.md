# Analyse du ZIP d'origine

## État constaté

- Aucun fichier QML, dossier `src/qt` ou `find_package(Qt6)` dans le ZIP.
- CMake C++20, GLFW, Vulkan, Assimp, GLM ; ImGui v1.91.8 téléchargé via FetchContent.
- `main → Application → VulkanContext → VulkanDevice/Swapchain/Renderer → NeuroUI`.
- BrainMesh charge et fusionne les quatre maillages du GLB avec prétransformation Assimp,
  centre et met à l'échelle la géométrie, puis crée buffers et pipeline graphique.
- Camera fournit une orbite, mais sa projection dépendait du réglage implicite de profondeur GLM.
- Shaders : éclairage, Fresnel, détails de surface, quatre foyers animés, scintillements.
- Le README décrivait déjà Qt, contrairement au programme réellement compilé.
- Des caches Visual Studio et des builds Windows étaient inclus dans l'archive.

## Problèmes corrigés ou évités par la migration

- L'ancien drawFrame ne recréait pas la swapchain en cas de VK_ERROR_OUT_OF_DATE_KHR.
- L'ancien CMake inscrivait les chemins absolus source/build dans l'exécutable.
- Les widgets de démonstration affichaient des couches/informations non associées à des données.
- La direction de vue du shader était fixe et ne suivait pas l'orbite de caméra.
- L'alpha minimum empêchait une transparence totale.
- Un échec de création du module fragment pouvait laisser le module vertex alloué.

## Choix

Conserver le modèle, l'import et la structure de dessin BrainMesh, améliorer Camera et GLSL.
Remplacer la couche de présentation par Qt Quick et le rendu dans une texture Vulkan importée.
Archiver l'ancien frontend après vérification de ses dépendances ; exclure ses sources et dépendances
du nouveau CMake. Les caches et binaires de build ne sont pas des sources et ne sont pas livrés.

Les priorités restent le fonctionnement du viewport, la stabilité et la séparation des threads.
La migration ne prétend pas résoudre l'EEG, la segmentation ni la transparence volumétrique.
