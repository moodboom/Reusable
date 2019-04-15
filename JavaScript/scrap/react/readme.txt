Trying out React + Typescript + Bootstrap + react-jsonschema-form.

Process:

npx create-react-app cra-bootstrap-jsonforms --typescript
cd cra-bootstrap-jsonforms
yarn start # and then you can start live-editing changes

# work through create-react-app/docs...
# https://facebook.github.io/create-react-app/docs

# eventually you'll get to...

# https://facebook.github.io/create-react-app/docs/adding-bootstrap
# npm install --save bootstrap
yarn add boostrap

# reactstrap turns bootstrap into usable with react
# ok... so is react goign to behave with other javascript libraries...?
npm install --save reactstrap@next

# import Bootstrap CSS in the src/index.js file:
import 'bootstrap/dist/css/bootstrap.css';

# https://react-jsonschema-form.readthedocs.io/en/latest/
npm install react-jsonschema-form --save