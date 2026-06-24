function textField(id, name, passwd=false) {
    /*
    * This function is part of a utility passed through to the ejs renderer.
    *
    * This function will return the html for a floating textField with the correct values for id, name and type.
    */

    return `
<div class="text-field">
    <input type="${passwd ? 'password' : 'text'}" id="${id}" name="${id}" placeholder=" " required="">

    <label for="${id}">${name}</label>

    <fieldset>
        <legend>${name}</legend>
    </fieldset>
</div>
    `;
}

export default { textField };