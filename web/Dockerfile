FROM node:lts-alpine
WORKDIR /app
COPY . .
EXPOSE 80
RUN npm install
CMD ["npm", "run", "start"]