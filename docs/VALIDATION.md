# Validation de la livraison — 26 septembre 2026

## Environnement effectivement utilisé

Ubuntu 24.04, GCC 13.3, CMake 3.28.3, Qt 6.4.2, Assimp 5.3.1, GLM,
Vulkan loader 1.3.275, glslc, Mesa llvmpipe (LLVM 20.1.2), Xvfb.
Il s'agit d'un **pilote Vulkan logiciel**, pas d'une exécution sur la RTX 4060.

## Résultats

| Vérification | Résultat |
|---|---|
| Configuration CMake neuve | Réussie |
| Compilation C++20 Debug | Réussie, sans avertissement C++ dans le build final |
| Compilation des deux shaders GLSL vers SPIR-V | Réussie |
| CTest CameraMath | 1/1 réussi |
| Chargement GLB | 195 887 sommets, 377 701 triangles |
| Affichage réel dans Qt Quick | Vérifié par captures |
| Événements bouton gauche, mouvement, relâchement, molette | Caméra passée à yaw −28°, pitch −1°, distance 2,83 |
| Transparence et luminosité | Changements visibles dans les captures |
| Palette ambre / recherche temporelle | Changement visible après réglage |
| Redimensionnement 1540×960 → 1120×760 | Vérifié |
| Masquage, libération du scene graph, réaffichage | Modèle et ressources Vulkan recréés, sans plantage |
| Surface masquée | Cerveau absent du viewport, UI conservée |
| Échelle d'affichage 150 % | Rendu et resize contrôlés visuellement |
| Modèle volontairement absent | Message d'erreur visible, pas de crash, retour attendu 3 |
| Validation Vulkan standard | Aucune erreur de validation dans les tests finaux |
| Windows/MSVC + Qt 6.11.2 + RTX 4060 | Non exécuté : à valider sur le PC cible |
| macOS/MoltenVK | Non exécuté |

Les captures sont issues de l'exécutable compilé. `apercu.png` montre le modèle réel.
Le smoke test utilise des étapes séquentielles espacées après chaque capture pour laisser
les événements et layouts se mettre à jour, y compris sur le pilote logiciel en HiDPI.

## Validation de synchronisation approfondie : réserve explicite

Avec `VK_LAYER_ENABLES=VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT`,
les couches signalent des hazards WRITE_AFTER_READ / WRITE_AFTER_PRESENT autour de la
swapchain Qt et de certains transferts de buffers Qt lors des captures/redimensionnements.

Une application **Qt Quick seule**, sans BrainMesh, sans texture importée et sans commandes
Vulkan NeuroScan, reproduit les alertes de transition de la swapchain. Cela isole ces alertes
de présentation du renderer personnalisé dans cet environnement Qt 6.4.2 / llvmpipe.
Les transferts de buffers mentionnés sont également des opérations Qt : le renderer
NeuroScan utilise des buffers statiques mappés et des push constants, sans vkCmdCopyBuffer
ni buffer uniforme dans sa boucle de dessin. Tous les diagnostics ne sont néanmoins pas
certifiés résolus : la validation avancée complète reste à refaire avec Qt 6.11.2 et le pilote cible.

Les journaux `validation/synchronisation-neuroscan.log` et `validation/qt-only-baseline.log`
conservent ces résultats. Aucun filtre ne masque ces avertissements dans l'application.
Ne pas confondre l'absence d'erreurs en validation standard avec une certification exhaustive
contre les hazards, ni le succès du smoke test avec une garantie de compilation Windows.

## Limites de la couverture

Pas de mesure de FPS ou de consommation mémoire sur la RTX 4060, ni de test longue durée,
ni de validation scientifique du modèle. Les tests ne prouvent ni la justesse anatomique
ni la possibilité de déduire une activité neuronale de données EEG.

La transparence alpha actuelle n'est pas un algorithme OIT. Un chargement asynchrone,
une mémoire GPU device-local et un système atlas/régions restent des étapes ultérieures.
