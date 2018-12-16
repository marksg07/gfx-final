To Run:
    Run ./CS123
    Open `scene.xml`


Design Choices and Features:
    1. We have a FEM physics based simluation to simulate soft body physics.
    2. We have shadow mapping (both point and directional)
    3. We have environment mapping
    3. We are using FXAA

We are using an RK4 integrator to minimize error in approximation.
We are using a penalty force linear to the penetration distance for collisions with the floor.

We decided not to multi-thread because it was slower due to overhead.
We did not have the time to implement friction or mesh-to-mesh collision.


Bugs:
    1. Objects can invert if given incorrect parameters (not really a bug).
    2. There is occasional frame lag, which appears to be a QT event loop issue.
