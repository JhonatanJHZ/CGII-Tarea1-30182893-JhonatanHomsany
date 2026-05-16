#pragma once

class Mesh{
    public:

    Mesh();
    ~Mesh();

    void centerMesh();
    void setupMesh(const std::vector<Vertex>& vertices);
};