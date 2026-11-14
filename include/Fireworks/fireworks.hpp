#pragma once
namespace NewYears::fireworks {
    void Create();
    void Destroy();
    // Discard stored controller references without touching Beat Saber's native
    // results-screen fireworks state.
    void Forget();
}
