# Bring up the containers (build if necessary)
up:
    @echo "Bringing up the containers..."
    docker-compose up --build -d

# Stop the containers
down:
    @echo "Stopping the containers..."
    docker-compose down

# Restart the containers
restart:
    @echo "Restarting the containers..."
    docker-compose down && docker-compose up --build -d

# Show the logs of the containers
logs:
    @echo "Showing logs..."
    docker-compose logs -f

# Access the shell of a specific container (default: model-server)
shell container="model-server":
    @echo "Opening shell of the {{container}} container..."
    docker-compose exec {{container}} bash

# Check the status of the containers
status:
    @echo "Checking container status..."
    docker-compose ps

# Clean up containers, volumes, and networks
clean:
    @echo "Cleaning up containers, volumes, and networks..."
    docker-compose down -v --rmi all --remove-orphans

