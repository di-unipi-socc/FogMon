#!/bin/bash
export COMPOSE_PROJECT_NAME=interface
case "$1" in
    "build")
        docker-compose build
        ;;
    "remove")
        docker-compose down -v
        ;;
    "down")
        docker-compose down
        ;;
    "swap")
        PREVIOUS_CONTAINER=$(docker ps --format "table {{.ID}}  {{.Names}}  {{.CreatedAt}}" | grep backend | awk -F  "  " '{print $1}')
        docker-compose up -d --no-deps --scale backend=2 --build --no-recreate backend
        sleep 15
        docker kill -s SIGINT $PREVIOUS_CONTAINER
        sleep 1
        docker rm -f $PREVIOUS_CONTAINER
        docker-compose up -d --no-deps --scale backend=1 --no-recreate backend
        docker-compose logs -f
        ;;
    *)
        docker-compose build
        docker-compose down
        docker-compose -f docker-compose-cleanup.yml down -v
        docker-compose up -d
        docker-compose logs -f
        ;;
esac