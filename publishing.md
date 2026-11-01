# Publish the required GitHub repository

The complete local project is prepared under the required name
`airport-bst-cpp`. Public publication has not been performed.

1. Sign in to your intended GitHub account and create a **public** repository
   named **airport-bst-cpp**. Start with an empty repository because this project
   already includes its README and .gitignore.
2. Upload the project contents: `src/`, `data/`, `docs/`, `tests/`, `scripts/`,
   `README.md`, `DATA_LICENSE.md`, `CMakeLists.txt` and `.gitignore`.
   Keep the GIF and the original data credit. The package's Windows executables
   in `bin/` are for convenience and are excluded by `.gitignore`.
3. Confirm GitHub displays the README, that `docs/demo.gif` animates, and that
   the repository visibility is Public. Submit the repository URL as requested
   by the course.

If using Git locally, open a terminal in this project root:

```sh
git init
git add .
git commit -m "Implement airport BST lookup service and experiments"
git branch -M main
git remote add origin https://github.com/YOUR_ACCOUNT/airport-bst-cpp.git
git push -u origin main
```

Replace `YOUR_ACCOUNT` with your real GitHub username. These commands assume an
empty remote repository and working GitHub authentication. `tests/tmp/`, build
outputs, and Windows executables are ignored; source data and evidence remain
included. Do not upload the compiler toolchain used to build the project.
