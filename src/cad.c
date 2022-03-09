#include "cad.h"

#include "raymath.h"
#include "rayext.h"

float BoxMass(Box box) {
    if (box.HardcodedMass) {
        return box.HardcodedMass;
    } else {
        return box.Width * box.Height * box.Depth;
    }
}

void UpdatePartCOM(Part *part) {
    float totalMass = 0;
    for (int i = 0; i < part->NumBoxes; i++) {
        Box box = part->Boxes[i];
        totalMass += BoxMass(box);
    }

    Vector3 COM = {0};
    for (int i = 0; i < part->NumBoxes; i++) {
        Box box = part->Boxes[i];
        float mass = BoxMass(box);
        float weight = mass / totalMass;

        COM = Vector3Add(COM, Vector3Scale(box.Translation, weight));
    }

    part->CenterOfMass = COM;
}
