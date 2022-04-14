FROM python:3.6.8-alpine3.9

ENV GROUP_ID=1000 \
    USER_ID=1000

WORKDIR /var/www/

RUN apk add --update --no-cache gcc musl-dev python3-dev

COPY requirements.txt /var/www/
RUN pip install -r requirements.txt --no-binary=pymongo
RUN pip install gunicorn

RUN addgroup -g $GROUP_ID www
RUN adduser -D -u $USER_ID -G www www -s /bin/sh


ADD . /var/www/
RUN chown www:www -R /var/www

USER www

EXPOSE 5000


CMD [ "gunicorn", "-w", "3", "--timeout", "600", "--bind", "0.0.0.0:5000", "wsgi"]
