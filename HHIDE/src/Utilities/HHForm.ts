interface HHForm{
    selector: string;
    closeForm();
}

interface HHReactForm{
    closeForm()
    render()
}

export {HHForm, HHReactForm}