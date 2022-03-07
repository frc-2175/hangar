#include "cad.h"

// TODO: Surely there are math functions _somewhere_

Vector3 AddVec3(Vector3 a, Vector3 b) {
    return (Vector3){ a.x + b.x, a.y + b.y, a.z + b.z };
}

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

        COM = AddVec3(COM, (Vector3){ box.X*weight, box.Y*weight, box.Z*weight });
    }

    part->CenterOfMass = COM;
}
