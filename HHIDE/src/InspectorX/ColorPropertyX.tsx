import * as React from "react"
import reactCss from "reactcss"
import {SketchPicker} from 'react-color'
import {Property} from "hhcommoncomponents"
import {SetFieldValueCommand} from "../RedoUndo/SetFieldValueCommand";
import {undoManager} from "../RedoUndo/UndoManager";
import {eps, PropertyEntry, PropertyProps, registerPropertyChangeListener} from "./BasePropertyX";
import {PropertyChangeListener} from "./PropertyChangeListener";

function isColorSame(paperColor, reactColor) {
    if (Math.abs(paperColor.red * 255 - Number(reactColor.r)) > eps
        || Math.abs(paperColor.green * 255 - Number(reactColor.g)) > eps
        || Math.abs(paperColor.blue * 255 - Number(reactColor.b)) > eps
        || Math.abs(paperColor.alpha - Number(reactColor.a)) > eps) {
        return false
    }

    return true
}

type ColorPropertyState = {
    displayColorPicker: boolean
    color: {
        r: number
        g: number
        b: number
        a: number
    }
}

class ColorPropertyX extends React.Component<PropertyProps, any> implements PropertyChangeListener{
    state: ColorPropertyState = {
        displayColorPicker: false,
        color: {
            r: 241,
            g: 112,
            b: 19,
            a: 1
        }
    }

    handleClick = () => {
        this.setState({displayColorPicker: !this.state.displayColorPicker})
    };

    handleClose = () => {
        this.setState({displayColorPicker: false})
    };

    handleChange = (color) => {
        let property = this.props.property

        this.state.color = color.rgb
        if (property.setter) {
            let oldColor = property.getter()
            let newColor = new paper.Color(color.hex)
            newColor.alpha = color.rgb.a

            let setColorCommand = new SetFieldValueCommand<paper.Color>(property.setter, oldColor, newColor)
            setColorCommand.DoCommand()
            undoManager.PushCommand(setColorCommand)
        }

        this.setState(this.state)
    };


    render() {
        let property = this.props.property

        registerPropertyChangeListener(this, property)

        let curColor = property.getter()
        if (!isColorSame(curColor, this.state.color)) {
            this.state.color.r = curColor.red * 255;
            this.state.color.g = curColor.green * 255;
            this.state.color.b = curColor.blue * 255;
            this.state.color.a = curColor.alpha;
        }

        const styles = reactCss({
            'default': {
                color: {
                    width: '36px',
                    height: '14px',
                    borderRadius: '2px',
                    background: `rgba(${this.state.color.r}, ${this.state.color.g}, ${this.state.color.b}, ${this.state.color.a})`,
                },
                swatch: {
                    padding: '5px',
                    background: '#fff',
                    borderRadius: '1px',
                    boxShadow: '0 0 0 1px rgba(0,0,0,.1)',
                    display: 'inline-block',
                    cursor: 'pointer',
                },
                popover: {
                    position: 'absolute',
                    zIndex: '2',
                },
                cover: {
                    position: 'fixed',
                    top: '0px',
                    right: '0px',
                    bottom: '0px',
                    left: '0px',
                },
            },
        })

        return (
            <PropertyEntry property={property}>
                <div style={styles.swatch} onClick={this.handleClick}>
                    <div style={styles.color}/>
                </div>
                {this.state.displayColorPicker ? <div style={styles.popover}>
                    <div style={styles.cover} onClick={this.handleClose}/>
                    <SketchPicker color={this.state.color} onChange={this.handleChange}/>
                </div> : null}

            </PropertyEntry>
        )
    }

    onValueChanged(val: any): void {
    }
}

export {ColorPropertyX}