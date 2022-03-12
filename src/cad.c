#include "cad.h"

#include <stdio.h>
#include "raymath.h"
#include "rayext.h"

Vector3 World2Part(Part part, Vector3 v) {
    return Vector3Subtract(v, part.Position);
    // TODO: Rotation
}

Vector3 Part2World(Part part, Vector3 v) {
    return Vector3Add(v, part.Position);
    // TODO: Rotation
}

float BoxMass(Box box) {
    if (box.HardcodedMass) {
        return box.HardcodedMass;
    } else {
        return box.Width * box.Height * box.Depth;
    }
}

void UpdateReferences(Part *part) {
    for (int b = 0; b < part->NumBoxes; b++) {
        Box *box = &part->Boxes[b];
        box->Part = part;
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

        Vector3 worldPos = Part2World(*part, box.Position);

        COM = Vector3Add(COM, Vector3Scale(worldPos, weight));
    }

    part->CenterOfMass = COM;
}
