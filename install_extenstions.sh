#!/bin/bash
# List of VS Code extensions to install
extensions=(
  ms-dotnettools.csharp
  esbenp.prettier-vscode
  ms-vscode.cpptools
)

# Install each extension
for extension in "${extensions[@]}"; do
  code --install-extension "$extension"
done

echo "All extensions have been installed."
