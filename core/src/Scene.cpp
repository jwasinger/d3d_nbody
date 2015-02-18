#include "Scene.h"

namespace Core
{
    void Scene::Scene()
    {
        this->renderer = new Renderer();
        this->objects = std::vector(10);
    }
    
    Scene::~Scene()
    {
        delete this->renderer;
    }

    void Scene::DrawScene()
    {
        this->begin_draw();

        for(int i = 0; i < this->objects.size(); i++)
        {
            this->objects[i].Render();
        }

        this->end_draw();
    }

    void Scene::begin_draw()
    {
        this->renderer->BeginRender();
    }

    void Scene::end_draw()
    {
        this->renderer->EndRender();
    }
}
