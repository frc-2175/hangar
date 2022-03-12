#include "cad.h"

#include <stdio.h>
#include "raymath.h"
#include "rayext.h"

Vector2 World2Part(Part part, Vector2 v) {
    return Vector2Rotate(Vector2Subtract(v, part.Position), -part.Angle * DEG2RAD);
}

Vector2 Part2World(Part part, Vector2 v) {
    return Vector2Add(Vector2Rotate(v, part.Angle * DEG2RAD), part.Position);
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

Vector2 ComputeCOM(Part *parts, int numParts) {
    float totalMass = 0;
    Vector2 COM = {0};

    for (int i = 0; i < numParts; i++) {
        Part *part = &parts[i];

        for (int i = 0; i < part->NumBoxes; i++) {
            Box box = part->Boxes[i];
            totalMass += BoxMass(box);
        }
    }

    for (int i = 0; i < numParts; i++) {
        Part *part = &parts[i];

        for (int i = 0; i < part->NumBoxes; i++) {
            Box box = part->Boxes[i];
            float mass = BoxMass(box);
            float weight = mass / totalMass;

            Vector2 worldPos = Part2World(*part, box.Position);

            COM = Vector2Add(COM, Vector2Scale(worldPos, weight));
        }
    }

    return COM;
}

void UpdatePartCOM(Part *part) {
    part->CenterOfMass = ComputeCOM(part, 1);
}
