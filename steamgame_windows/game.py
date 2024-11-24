import steam
import pygame

class Game:
    def __init__(self):
        pygame.init()
        
        self.clock = pygame.time.Clock()
        self.running = False
        
        self.screen = pygame.display.set_mode((192, 108), flags = pygame.SCALED)
        
    def update(self, dt):
        pass
        
    def draw(self, surface):
        surface.fill("darkblue")
        pygame.display.flip()
        
    def run(self):
        self.running = True
        
        while self.running:
            dt = self.clock.tick() * 0.001
            for event in pygame.event.get():
                if event.type == pygame.QUIT:
                    self.running = False
            
            self.update(dt)
            self.draw(self.screen)
            
if __name__ == '__main__':
    g = Game()
    g.run()