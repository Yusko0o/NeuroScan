import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import NeuroScan 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1540; height: 960
    minimumWidth: 1100; minimumHeight: 740
    title: "NeuroScan · Anatomical workspace"
    color: "#08111d"
    property color muted: "#8096ad"
    property color accent: "#56d9f5"
    property string clock: Qt.formatDateTime(new Date(), "dd MMM yyyy  ·  hh:mm:ss")
    font.family: "Segoe UI"
    font.pixelSize: 13
    palette.window: "#0d1928"
    palette.windowText: "#dce9f7"
    palette.base: "#142638"
    palette.text: "#dce9f7"
    palette.button: "#192d42"
    palette.buttonText: "#dce9f7"
    palette.highlight: accent
    palette.highlightedText: "#08111d"
    Timer { interval: 1000; running: true; repeat: true; onTriggered: root.clock = Qt.formatDateTime(new Date(), "dd MMM yyyy  ·  hh:mm:ss") }
    Shortcut { sequence: "Space"; onActivated: brain.playing = !brain.playing }
    Shortcut { sequence: "R"; onActivated: brain.setView(0) }

    component Caption: Label { color: root.muted; font.pixelSize: 11; font.letterSpacing: 1.4 }
    component Note: Label { color: root.muted; wrapMode: Text.WordWrap; lineHeight: 1.3; Layout.fillWidth: true }
    component Rule: Rectangle { color: "#203346"; Layout.fillWidth: true; implicitHeight: 1 }
    component Panel: Rectangle { color: "#0d1928"; border.color: "#213449"; radius: 8 }
    component ViewButton: Button { Layout.fillWidth: true; implicitHeight: 34 }

    ColumnLayout {
        anchors.fill: parent; anchors.margins: 16; spacing: 12
        RowLayout {
            Layout.fillWidth: true; Layout.preferredHeight: 62; spacing: 16
            Rectangle { width: 4; height: 36; radius: 2; color: root.accent }
            ColumnLayout {
                spacing: 2
                Label { text: "NEUROSCAN"; color: "#edf7ff"; font.pixelSize: 25; font.letterSpacing: 3; font.bold: true }
                Caption { text: "ANATOMICAL WORKSPACE  /  v0.2.0" }
            }
            Item { Layout.fillWidth: true }
            ColumnLayout { Caption { text: "SUJET / SESSION" } Label { text: "Aucun sujet · Démonstration locale"; color: "#dce9f7" } }
            Rectangle {
                Layout.leftMargin: 12; implicitWidth: 150; implicitHeight: 31; radius: 15
                color: "#312b1b"; border.color: "#776137"
                Label { anchors.centerIn: parent; text: "●  ACTIVITÉ SIMULÉE"; color: "#f3cc7e"; font.pixelSize: 10; font.bold: true }
            }
            Label { text: root.clock; color: root.muted; font.pixelSize: 11 }
        }
        RowLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 12
            Panel {
                Layout.preferredWidth: 226; Layout.fillHeight: true
                ScrollView {
                    id: leftScroll
                    anchors.fill: parent; anchors.margins: 16; clip: true
                    contentWidth: availableWidth
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ColumnLayout {
                        width: leftScroll.availableWidth - 12; spacing: 12
                        Caption { text: "01  /  VISUALISATION" }
                        Label { text: "Vues du modèle"; color: "#e0edf7"; font.pixelSize: 17 }
                        GridLayout {
                            columns: 2; Layout.fillWidth: true
                            ViewButton { text: "+Z"; onClicked: brain.setView(1) }
                            ViewButton { text: "+X"; onClicked: brain.setView(2) }
                            ViewButton { text: "+Y"; onClicked: brain.setView(3) }
                            ViewButton { text: "−Z"; onClicked: brain.setView(4) }
                        }
                        Note { text: "Axes du modèle. Orientation anatomique non recalée sur un atlas."; font.pixelSize: 11 }
                        ViewButton { text: "Réinitialiser  ·  R"; onClicked: brain.setView(0) }
                        Rule {}
                        RowLayout { Layout.fillWidth: true; Label { text: "Transparence" } Item { Layout.fillWidth: true } Label { text: Math.round(brain.transparency * 100) + "%"; color: root.accent } }
                        Slider { objectName: "transparencySlider"; Layout.fillWidth: true; from: 0; to: 1; value: brain.transparency; onMoved: brain.transparency = value }
                        RowLayout { Layout.fillWidth: true; Label { text: "Luminosité" } Item { Layout.fillWidth: true } Label { text: Math.round(brain.brightness * 100) + "%"; color: root.accent } }
                        Slider { objectName: "brightnessSlider"; Layout.fillWidth: true; from: 0; to: 1; value: brain.brightness; onMoved: brain.brightness = value }
                        Label { text: "Densité des scintillements" }
                        Slider { Layout.fillWidth: true; from: 0; to: 1; value: brain.density; onMoved: brain.density = value }
                        Note { text: "Effet visuel procédural, sans correspondance avec une densité neuronale."; font.pixelSize: 11 }
                        ComboBox { Layout.fillWidth: true; model: ["Palette · Cyan", "Palette · Ambre", "Palette · Monochrome"]; currentIndex: brain.colorScheme; onActivated: brain.colorScheme = currentIndex }
                        Rule {}
                        Caption { text: "COUCHES DISPONIBLES" }
                        CheckBox { text: "Surface du modèle"; checked: brain.surfaceVisible; onToggled: brain.surfaceVisible = checked }
                        CheckBox { text: "Activité procédurale"; checked: brain.activityEnabled; onToggled: brain.activityEnabled = checked }
                        Note { text: "Crâne, neurones, vaisseaux et connectome : données non fournies."; font.pixelSize: 11 }
                    }
                }
            }
            Panel {
                Layout.fillWidth: true; Layout.fillHeight: true
                ColumnLayout {
                    anchors.fill: parent; anchors.margins: 1; spacing: 0
                    RowLayout {
                        Layout.fillWidth: true; Layout.margins: 16
                        Caption { text: "02  /  VOLUME 3D" }
                        Item { Layout.fillWidth: true }
                        Label { text: "VULKAN  /  GLB"; color: root.accent; font.pixelSize: 10; font.letterSpacing: 1 }
                    }
                    Item {
                        Layout.fillWidth: true; Layout.fillHeight: true; clip: true
                        BrainViewport { id: brain; objectName: "brainViewport"; anchors.fill: parent }
                        Column {
                            anchors.top: parent.top; anchors.left: parent.left; anchors.margins: 20; spacing: 4
                            Label { text: "HUMAN BRAIN"; font.pixelSize: 12; font.letterSpacing: 2; color: "#7c9aaf" }
                            Label { text: brain.triangleCount.toLocaleString(Qt.locale(), "f", 0) + " triangles"; color: "#536d85"; font.pixelSize: 11 }
                        }
                        Label {
                            anchors.centerIn: parent; width: parent.width - 60; wrapMode: Text.WordWrap
                            horizontalAlignment: Text.AlignHCenter; color: "#f3cc7e"
                            visible: brain.status.indexOf("Erreur") >= 0 || brain.triangleCount === 0
                            text: brain.status
                        }
                        Label { anchors.bottom: parent.bottom; anchors.left: parent.left; anchors.margins: 16; text: brain.orientation; color: root.muted; font.pixelSize: 11 }
                        Row {
                            anchors.bottom: parent.bottom; anchors.right: parent.right; anchors.margins: 12; spacing: 5
                            Button { text: "−"; width: 35; onClicked: brain.zoom(-1) }
                            Button { text: "+"; width: 35; onClicked: brain.zoom(1) }
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true; Layout.margins: 14
                        Label { text: "Glisser : rotation    ·    Molette : zoom    ·    Double-clic : reset"; color: root.muted; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }
                    }
                }
            }
            Panel {
                Layout.preferredWidth: root.width < 1250 ? 225 : 270; Layout.fillHeight: true
                ScrollView {
                    id: rightScroll
                    anchors.fill: parent; anchors.margins: 16; clip: true; contentWidth: availableWidth
                    ScrollBar.vertical.policy: ScrollBar.AlwaysOn
                    ColumnLayout {
                        width: rightScroll.availableWidth - 12; spacing: 14
                        Caption { text: "03  /  INSPECTEUR" }
                        Label { text: "Surface cérébrale"; font.pixelSize: 17 }
                        Note { text: "Modèle chargé depuis brain.glb. La géométrie est normalisée pour la navigation." }
                        Rule {}
                        Caption { text: "RÉGION SÉLECTIONNÉE" }
                        Label { text: "Aucune région annotée"; color: "#e4bd79" }
                        Note { text: "La sélection anatomique nécessite un atlas et un recalage du modèle. Aucune région n’est déduite de sa seule position à l’écran."; font.pixelSize: 12 }
                        Rule {}
                        Caption { text: "ACTIVITY MONITOR" }
                        Label { text: brain.activityEnabled ? "Générateur procédural" : "Activité désactivée"; color: root.accent }
                        Note { text: "Quatre foyers GLSL et scintillements de surface. Les indicateurs ci-dessous montrent les enveloppes des foyers simulés, pas un signal EEG."; font.pixelSize: 12 }
                        Repeater {
                            model: 4
                            delegate: ColumnLayout {
                                required property int index
                                Layout.fillWidth: true; spacing: 4
                                property real pulse: brain.activityEnabled ? 0.5 + 0.5 * Math.sin(brain.simulationTime * [2.4, 1.8, 3.1, 2.0][index] + [0, 1.7, 3.4, 5.0][index]) : 0
                                RowLayout { Layout.fillWidth: true; Label { text: "Foyer simulé 0" + (index + 1); color: root.muted; font.pixelSize: 11 } Item { Layout.fillWidth: true } Label { text: pulse.toFixed(2); color: root.accent; font.pixelSize: 11 } }
                                ProgressBar { Layout.fillWidth: true; value: parent.pulse }
                            }
                        }
                        Rule {}
                        Caption { text: "QUALITÉ DES DONNÉES" }
                        Note { text: "Aucun capteur connecté\nAucune mesure acquise\nÉchelle physique non étalonnée"; font.pixelSize: 12 }
                    }
                }
            }
        }
        Panel {
            Layout.fillWidth: true; Layout.preferredHeight: 142
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 16; spacing: 10
                RowLayout {
                    Caption { text: "04  /  TEMPS DE SIMULATION" }
                    Item { Layout.fillWidth: true }
                    Label { text: "Boucle de démonstration · 120 s"; color: root.muted; font.pixelSize: 11 }
                }
                RowLayout {
                    spacing: 16; Layout.fillWidth: true
                    Button { text: brain.playing ? "Ⅱ  Pause" : "▶  Lecture"; implicitWidth: 120; onClicked: brain.playing = !brain.playing }
                    Button { text: "↺"; onClicked: { brain.simulationTime = 0; brain.playing = false } }
                    Slider { Layout.fillWidth: true; from: 0; to: 120; value: brain.simulationTime; onPressedChanged: { if (pressed) brain.playing = false } onMoved: brain.simulationTime = value }
                    Label { text: brain.simulationTime.toFixed(1) + " s"; color: root.accent; Layout.preferredWidth: 62 }
                }
                Rule {}
                RowLayout {
                    Layout.fillWidth: true
                    Label { text: brain.status; color: root.muted; font.pixelSize: 11; Layout.fillWidth: true; elide: Text.ElideRight }
                    Label { text: brain.deviceName; color: root.muted; font.pixelSize: 11; Layout.maximumWidth: 440; elide: Text.ElideRight }
                }
            }
        }
    }
}
