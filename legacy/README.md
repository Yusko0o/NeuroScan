# Référence de l'architecture d'origine

Sources GLFW/ImGui, shaders et CMake d'origine conservés pour comparaison.
Ils ne participent pas au build Qt et ne constituent pas une deuxième application maintenue.
`CMakeLists.original.txt` est une copie de référence, pas un sous-projet CMake.
Le GLB inchangé se trouve dans `../assets/models/brain.glb`.

Les classes VulkanContext/Device/Swapchain/Renderer d'origine géraient toute la fenêtre GLFW.
Elles dépendaient de l'ancienne UI et ne pouvaient pas devenir propriétaires des ressources Qt.
La gestion du device, des frames et de la présentation revient désormais à Qt ; BrainTextureNode
possède uniquement les ressources offscreen et BrainMesh les buffers/pipeline du cerveau.
