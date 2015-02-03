bool A2Application::Init(void)
{
    //map necessary controls
    ControlMapping control_mapping; 
    this->GetInputController().MapControls(control_mapping);
}

void A2Application::Render(void)
{
    //render lines
}

void A2Application::Update(void)
{
    double delta_ms = this->GetTimer().GetRealElapsed();

    //do something
}
