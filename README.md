# Space Bots

Space Bots is an early-stage RTS (Real-Time Strategy) game where players code their own units. The goal is to create a competitive and educational environment where coding skills directly influence gameplay.

## Features

- Program your own units with customizable logic.
- Engage in tactical battles controlled by your algorithms.
- Designed for tech-savvy players interested in coding and strategy.
- Early development — expect frequent changes and ongoing improvements.

## Build Dependencies

### Steamworks SDK

This project requires the Steamworks SDK, which is proprietary and must be downloaded separately from Valve's website:

https://partner.steamgames.com/downloads/steamworks_sdk

Place the SDK files in `libs/steam`.

The SDK is not included in this repository due to licensing restrictions.

### Rrespacker

This project requires Rrespacker, which is proprietary software and must be obtained separately:

https://raylibtech.itch.io/rrespacker

Place Rrespacker files in `client/libs/rrespacker`.

Rrespacker is not included in this repository due to licensing restrictions.

## Getting Started

Clone the repository and explore the code to learn how units are programmed and interact. You can build and run the game locally to experiment with your own unit AI.

```bash
git clone --recurse-submodules https://github.com/Liam-Weitzel/Space-Bots.git
cd space-bots/client
mkdir build
cd build
cmake ..
make
cd ..
./client
```

## License

This project is licensed under the [Creative Commons Attribution-NonCommercial 4.0 International (CC BY-NC 4.0)](https://creativecommons.org/licenses/by-nc/4.0/). You are free to view, learn, modify, and run the code locally, but **commercial use is prohibited**.

The repository includes git submodules with third-party code licensed separately under [MIT/GPL/etc.]. Please refer to that submodule’s license for details.

## Contributing

Contributions are welcome! By submitting code, you agree to license your contributions under the same non-commercial license. Please open issues or pull requests to discuss ideas or improvements.

## Contact

For questions or feedback, please open an issue or contact me (details in my bio).

## Disclaimer

Space Bots is in very early development. Features are subject to change, and the game is not yet ready for full release.

---

Thanks for checking out Space Bots
