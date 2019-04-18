Trying out React + Typescript + Bootstrap + react-jsonschema-form.

Process:

# Typescript seems to be the way to go, but not well supported in the create-react+++ process yet.
# npx create-react-app cra-bootstrap-jsonforms --typescript
# Default uses yarn, let's go back to npm instead
# npx create-react-app cra-bootstrap-jsonforms
npx create-react-app cra-bootstrap-jsonforms --use-npm
cd cra-bootstrap-jsonforms
npm start # and then you can start live-editing changes

# work through create-react-app/docs...
# https://facebook.github.io/create-react-app/docs

# https://facebook.github.io/create-react-app/docs/adding-bootstrap
# says to do this, but just move on to reactstrap instead...
# npm install --save bootstrap

# reactstrap turns bootstrap into usable with react
# ok... so is react goign to behave with other javascript libraries...?
npm install bootstrap --save
npm install --save reactstrap react react-dom

# import Bootstrap CSS in the src/index.js file by adding this AT THE TOP:
import 'bootstrap/dist/css/bootstrap.min.css';

# https://react-jsonschema-form.readthedocs.io/en/latest/
npm install react-jsonschema-form --save

# to directly read JSON
npm i json-loader --save

# attempt to patch in security fixes
npm audit fix

----------


# ONE LINER
npx create-react-app cra-bootstrap-jsonforms --use-npm && cd cra-bootstrap-jsonforms && npm install --save bootstrap reactstrap react react-dom react-jsonschema-form && npm audit fix && npm start

# import Bootstrap CSS in the src/index.js file:
import 'bootstrap/dist/css/bootstrap.min.css';
