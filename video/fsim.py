import pygame
import math
import sys

# Initialize Pygame
pygame.init()

# Screen dimensions and settings
WIDTH, HEIGHT = 800, 600
screen = pygame.display.set_mode((WIDTH, HEIGHT))
pygame.display.set_caption("3D Bitmap Renderer")
scale = 65535  # Scale of the 3D world

# Colors
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)

# Camera parameters
camera_pos = [0, 0, scale * 1/8]  # Start at scale * 1/8 above the plane
camera_yaw = 0          # Rotation around the Y-axis
camera_pitch = 0        # Rotation around the X-axis
camera_roll = 0         # Rotation around the Z-axis
camera_speed = 0        # Speed of camera movement
max_speed = 255         # Max camera speed
min_speed = 0           # Min camera speed

# Load bitmap texture
bitmap = pygame.image.load("/home/smith/Agon/mystuff/pingoasm/src/blender/koak.png")  # Use specified file path
bitmap_size = bitmap.get_width()  # Base size of the bitmap from its width
bitmap_scale = 1.0      # Scale of the bitmap (adjustable)
world_size = bitmap_size * scale  # Determine the world size based on pixel dimensions

def draw_textured_quad(screen, camera_pos, camera_yaw, camera_pitch, camera_roll, bitmap, scale):
    # Define quad vertices in 3D space (a flat plane on the x, z plane)
    size = bitmap.get_width() * scale  # Scale bitmap size to world size
    quad_vertices = [
        [-size / 2, 0, -size / 2],  # Top-left
        [size / 2, 0, -size / 2],   # Top-right
        [size / 2, 0, size / 2],    # Bottom-right
        [-size / 2, 0, size / 2]    # Bottom-left
    ]

    # Apply camera transformations (yaw, pitch, roll)
    transformed_vertices = []
    for v in quad_vertices:
        # Rotate around Y-axis (Yaw)
        x = v[0] * math.cos(math.radians(camera_yaw)) - v[2] * math.sin(math.radians(camera_yaw))
        z = v[0] * math.sin(math.radians(camera_yaw)) + v[2] * math.cos(math.radians(camera_yaw))

        # Rotate around X-axis (Pitch)
        y = v[1] * math.cos(math.radians(camera_pitch)) - z * math.sin(math.radians(camera_pitch))
        z = v[1] * math.sin(math.radians(camera_pitch)) + z * math.cos(math.radians(camera_pitch))

        # Rotate around Z-axis (Roll)
        x = x * math.cos(math.radians(camera_roll)) - y * math.sin(math.radians(camera_roll))
        y = x * math.sin(math.radians(camera_roll)) + y * math.cos(math.radians(camera_roll))

        # Apply camera position
        x += camera_pos[0]
        y += camera_pos[1]
        z += camera_pos[2]

        # Perspective projection
        if z != 0:
            f = 200 / z  # Focal length or perspective scaling factor
            screen_x = int(WIDTH / 2 + f * x)
            screen_y = int(HEIGHT / 2 - f * y)
            transformed_vertices.append((screen_x, screen_y))

    # Draw the quad if all four vertices are visible
    if len(transformed_vertices) == 4:
        # Draw textured quad
        # Calculate the bounding box for the transformed quad
        min_x = min(v[0] for v in transformed_vertices)
        max_x = max(v[0] for v in transformed_vertices)
        min_y = min(v[1] for v in transformed_vertices)
        max_y = max(v[1] for v in transformed_vertices)
        
        # Calculate width and height of the bounding box
        width = max_x - min_x
        height = max_y - min_y
        
        # Transform the bitmap to fit the bounding box
        transformed_bitmap = pygame.transform.scale(bitmap, (width, height))
        
        # Blit the transformed bitmap to the screen
        screen.blit(transformed_bitmap, (min_x, min_y))

def handle_input():
    global camera_speed, camera_yaw, camera_pitch, camera_roll

    keys = pygame.key.get_pressed()

    # Adjust camera speed
    if keys[pygame.K_w]:
        camera_speed = min(max_speed, camera_speed + 1)
    if keys[pygame.K_s]:
        camera_speed = max(min_speed, camera_speed - 1)

    # Adjust camera yaw (Y-axis rotation)
    if keys[pygame.K_a]:
        camera_yaw -= 2
    if keys[pygame.K_d]:
        camera_yaw += 2

    # Adjust camera roll (Z-axis rotation)
    if keys[pygame.K_LEFT]:
        camera_roll -= 2
    if keys[pygame.K_RIGHT]:
        camera_roll += 2

    # Adjust camera pitch (X-axis rotation)
    if keys[pygame.K_UP]:
        camera_pitch -= 2
    if keys[pygame.K_DOWN]:
        camera_pitch += 2

def main():
    global camera_pos

    clock = pygame.time.Clock()

    # Main loop
    while True:
        # Handle events
        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                pygame.quit()
                sys.exit()

        # Handle input
        handle_input()

        # Update camera position
        # Move forward based on speed and yaw
        camera_pos[0] -= camera_speed * math.sin(math.radians(camera_yaw)) * 0.1
        camera_pos[2] -= camera_speed * math.cos(math.radians(camera_yaw)) * 0.1

        # Clear screen
        screen.fill(BLACK)

        # Draw the textured quad (bitmap)
        draw_textured_quad(screen, camera_pos, camera_yaw, camera_pitch, camera_roll, bitmap, bitmap_scale)

        # Update display
        pygame.display.flip()

        # Cap the frame rate
        clock.tick(60)

if __name__ == "__main__":
    main()
