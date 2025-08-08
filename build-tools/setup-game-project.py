#!/usr/bin/env python3
"""
DotBlue Game Project Generator
Creates a new game project with proper DotBlue integration
"""

import os
import sys
import shutil
from pathlib import Path

def create_game_project(game_name, project_dir):
    """Create a new game project from template"""
    
    # Convert to proper paths
    project_path = Path(project_dir) / game_name
    template_path = Path(__file__).parent.parent / "game-template"
    
    if project_path.exists():
        print(f"Error: Project directory {project_path} already exists!")
        return False
    
    print(f"Creating game project '{game_name}' in {project_path}")
    
    try:
        # Copy template
        shutil.copytree(template_path, project_path)
        
        # Update CMakeLists.txt with project name
        cmake_file = project_path / "CMakeLists.txt"
        with open(cmake_file, 'r') as f:
            content = f.read()
        
        content = content.replace("PlatformerGame", game_name)
        
        with open(cmake_file, 'w') as f:
            f.write(content)
        
        # Update main.cpp with project name
        main_file = project_path / "src" / "main.cpp"
        with open(main_file, 'r') as f:
            content = f.read()
        
        content = content.replace("PlatformerGame", f"{game_name}Game")
        
        with open(main_file, 'w') as f:
            f.write(content)
        
        # Create basic directory structure
        (project_path / "assets" / "textures").mkdir(parents=True, exist_ok=True)
        (project_path / "assets" / "sounds").mkdir(parents=True, exist_ok=True)
        (project_path / "assets" / "shaders").mkdir(parents=True, exist_ok=True)
        (project_path / "build").mkdir(exist_ok=True)
        
        print(f"✅ Game project '{game_name}' created successfully!")
        print(f"📁 Location: {project_path}")
        print("\n🚀 Next steps:")
        print(f"   cd {project_path}")
        print("   mkdir build && cd build")
        print("   cmake ..")
        print("   cmake --build .")
        
        return True
        
    except Exception as e:
        print(f"❌ Error creating project: {e}")
        return False

def main():
    if len(sys.argv) != 3:
        print("Usage: python setup-game-project.py <game_name> <projects_directory>")
        print("Example: python setup-game-project.py MyAwesomeGame ../games/")
        sys.exit(1)
    
    game_name = sys.argv[1]
    project_dir = sys.argv[2]
    
    create_game_project(game_name, project_dir)

if __name__ == "__main__":
    main()
